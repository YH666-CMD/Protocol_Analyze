/*
 * app_usb_comm.c
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 协议任务。
 */
#include "app_usb_comm.h"
#include "config.h"
#include "drv_usb.h"
#include "drv_uart.h"
#include "../../03_Driver/adc/drv_adc.h"
#include "../../03_Driver/spi/drv_spi.h"
#include "../../03_Driver/i2c/drv_i2c.h"
#include "../../03_Driver/pwm_in/drv_pwm_in.h"
#include "../../03_Driver/pwm_out/drv_pwm_out.h"
#include "../spi_analyze/app_spi_analyze.h"
#include "frame.h"
#include "ring_buffer.h"
#include "usb_cfg.h"
#include "usb_log.h"
#if (USB_LOG_ENABLE != 0u) && (PRINT_ENABLE != 0u)
#include "print.h"
#endif
#include <stddef.h>
#include "ota_boot.h"
#include "bsp_flash.h"
#include "bsp_time.h"
#include "crc.h"
#include "aes.h"
#include "log.h"
#include "ch32v20x.h"

#if (FRAME_MAX > USB_EP_MPS)
#error "FRAME_MAX must not exceed USB_EP_MPS"
#endif

#define USB_UP_FRAME_MAX     8u
#define USB_DOWN_FRAME_MAX   8u

typedef result_t (*usb_cmd_fn)(uint8_t cmd, const uint8_t *data, uint16_t len);
typedef result_t (*usb_ctrl_fn)(const uint8_t *arg, uint16_t arg_len);

/* ack 为 1 时，分发按返回值回 ACK 或 NACK。为 0 时处理函数自己决定。 */
typedef struct
{
    uint8_t    cmd;
    uint8_t    ack;
    usb_cmd_fn pf_on;
} usb_cmd_row_t;

/* 控制动作表。以后关 PWM、开 ADC，在这里加一行。 */
typedef struct
{
    uint8_t     mod;
    uint8_t     op;
    usb_ctrl_fn pf_on;
} usb_ctrl_row_t;

/* 波特率功能码表。码和 frame.h 里的 FRAME_BAUD_* 对应，加波特率只加一行。 */
typedef struct
{
    uint8_t  code;
    uint32_t baud;
} usb_baud_row_t;

static rb_t s_usb_rx;

static void usb_on_rx(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0u)
    {
        return;
    }
    (void)rb_sw_driver.pf_rb_write_buf(&s_usb_rx, data, len);
}

result_t app_usb_comm_report(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint8_t pkt[FRAME_MAX];
    uint16_t packed;
    uint16_t room;

    room = drv_usb.pf_tx_ready();
    if (room < FRAME_MIN)
    {
        return RES_BUSY;
    }
    if (frame_sw_driver.pf_pack(cmd, data, len, pkt, (uint16_t)sizeof(pkt), &packed) != RES_OK)
    {
        return RES_PARAM;
    }
    if (packed > room)
    {
        return RES_BUSY;
    }
    return drv_usb.pf_tx(pkt, packed);
}

static void usb_reply(uint8_t ack_cmd, uint8_t req, result_t rc)
{
    uint8_t body[2];

    body[0] = req;
    body[1] = (uint8_t)rc;
    (void)app_usb_comm_report(ack_cmd, body, 2u);
}

static result_t usb_on_ping(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    (void)cmd;
    (void)data;
    (void)len;
    return RES_OK;
}

static const usb_baud_row_t s_baud[] =
{
    { FRAME_BAUD_115200, 115200u },
    { FRAME_BAUD_9600,    9600u   },
    { FRAME_BAUD_19200,   19200u  },
    { FRAME_BAUD_38400,   38400u  },
    { FRAME_BAUD_57600,   57600u  },
    { FRAME_BAUD_230400,  230400u },
    { FRAME_BAUD_460800,  460800u },
    { FRAME_BAUD_921600,  921600u },
};

static uint32_t usb_baud_of(uint8_t code, uint8_t *ok)
{
    uint8_t i;

    for (i = 0u; i < (uint8_t)(sizeof(s_baud) / sizeof(s_baud[0])); i++)
    {
        if (s_baud[i].code == code)
        {
            *ok = 1u;
            return s_baud[i].baud;
        }
    }
    *ok = 0u;
    return 0u;
}

static result_t usb_on_uart_tx(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint16_t free_n;
    result_t rc;

    (void)cmd;
    if (len == 0u)
    {
        return RES_OK;
    }
    if (data == NULL)
    {
        return RES_PARAM;
    }

    free_n = drv_uart.pf_tx_free();
    if (free_n == 0u)
    {
        return RES_NO_MEM;
    }
    if (len > free_n)
    {
        len = free_n;
        rc = RES_NO_MEM;
    }
    else
    {
        rc = RES_OK;
    }

    for (i = 0u; i < len; i++)
    {
        if (drv_uart.pf_putc(data[i]) != RES_OK)
        {
            return RES_NO_MEM;
        }
    }
    return rc;
}

static result_t usb_on_uart_line(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uart_line_t line;
    uint8_t ok;
    uint8_t stop;
    uint8_t parity;
    uint8_t bits;

    (void)cmd;
    if (data == NULL || (len != 1u && len != 4u))
    {
        return RES_PARAM;
    }
    if (drv_uart.pf_line_get(&line) != RES_OK)
    {
        return RES_STATE;
    }

    line.baud = usb_baud_of(data[0], &ok);
    if (ok == 0u)
    {
        return RES_PARAM;
    }
    if (len == 1u)
    {
        return drv_uart.pf_line_set(&line);
    }

    stop = data[1];
    parity = data[2];
    bits = data[3];
    if (stop == FRAME_STOP_1)
    {
        line.stop_bits = UART_STOP_BITS_1;
    }
    else if (stop == FRAME_STOP_1_5)
    {
        line.stop_bits = UART_STOP_BITS_1_5;
    }
    else if (stop == FRAME_STOP_2)
    {
        line.stop_bits = UART_STOP_BITS_2;
    }
    else
    {
        return RES_PARAM;
    }

    if (parity == FRAME_PAR_NONE)
    {
        line.parity = UART_PARITY_NONE;
    }
    else if (parity == FRAME_PAR_ODD)
    {
        line.parity = UART_PARITY_ODD;
    }
    else if (parity == FRAME_PAR_EVEN)
    {
        line.parity = UART_PARITY_EVEN;
    }
    else
    {
        return RES_PARAM;
    }

    if (bits == FRAME_BITS_7)
    {
        line.data_bits = UART_DATA_BITS_7;
    }
    else if (bits == FRAME_BITS_8)
    {
        line.data_bits = UART_DATA_BITS_8;
    }
    else if (bits == FRAME_BITS_9)
    {
        line.data_bits = UART_DATA_BITS_9;
    }
    else
    {
        return RES_PARAM;
    }

    return drv_uart.pf_line_set(&line);
}

#if (OTA_FLAG_DBG != 0u)

#define USB_FLAG_SKIP   (offsetof(ota_flag_t, run_slot))
#define USB_FLAG_COPY   (offsetof(ota_flag_t, crc) - offsetof(ota_flag_t, run_slot))

typedef char usb_flag_fit_check[((4u + USB_FLAG_COPY) <= FRAME_DATA_MAX) ? 1 : -1];

static void usb_move(uint8_t *dst, const uint8_t *src, uint32_t n)
{
    uint32_t i;

    for (i = 0u; i < n; i++)
    {
        dst[i] = src[i];
    }
}

/* 先回一帧 FLAG：seq 小端，后面 52 字节从 run_slot 开始，不含记录自己的 CRC。 */
static result_t usb_flag_rd(const uint8_t *arg, uint16_t arg_len)
{
    ota_flag_t rec;
    uint8_t body[4u + USB_FLAG_COPY];
    result_t rc;

    (void)arg;
    if (arg_len != 0u)
    {
        return RES_PARAM;
    }
    rc = ota_boot.pf_load(&rec);
    if (rc != RES_OK)
    {
        return rc;
    }
    body[0] = (uint8_t)rec.seq;
    body[1] = (uint8_t)(rec.seq >> 8);
    body[2] = (uint8_t)(rec.seq >> 16);
    body[3] = (uint8_t)(rec.seq >> 24);
    usb_move(&body[4], ((const uint8_t *)&rec) + USB_FLAG_SKIP, USB_FLAG_COPY);
    return app_usb_comm_report(FRAME_CMD_FLAG, body, (uint16_t)sizeof(body));
}

static result_t usb_flag_wr(const uint8_t *arg, uint16_t arg_len)
{
    ota_flag_t rec;

    if ((arg == 0) || (arg_len != USB_FLAG_COPY))
    {
        return RES_PARAM;
    }
    rec.magic = 0u;
    rec.seq = 0u;
    rec.crc = 0u;
    usb_move(((uint8_t *)&rec) + USB_FLAG_SKIP, arg, USB_FLAG_COPY);
    return ota_boot.pf_save(&rec);
}

static result_t usb_flag_clr(const uint8_t *arg, uint16_t arg_len)
{
    (void)arg;
    if (arg_len != 0u)
    {
        return RES_PARAM;
    }
    return bsp_flash.pf_erase(MEM_FLASH_PHY_BASE + MEM_FLAG_OFS, MEM_FLAG_SIZE);
}

#endif

/* 串口上报默认开。关只停上报，不关 USB，也不关接收。 */
static uint8_t s_uart_report = 1u;
static uint8_t s_ota_on;

static result_t usb_sw_len(const uint8_t *arg, uint16_t arg_len)
{
    (void)arg;
    if (arg_len != 0u)
    {
        return RES_PARAM;
    }
    return RES_OK;
}

static void usb_biz_halt(void)
{
    s_uart_report = 0u;
    drv_adc.pf_stop();
    drv_spi.pf_stop();
    drv_i2c.pf_stop();
    drv_pwm_out.pf_stop();
    drv_pwm_in.pf_stop();
}

static result_t usb_uart_on(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (s_ota_on != 0u)
    {
        return RES_STATE;
    }
    s_uart_report = 1u;
    return RES_OK;
}

static result_t usb_uart_off(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    s_uart_report = 0u;
    return RES_OK;
}

static result_t usb_adc_on(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (s_ota_on != 0u)
    {
        return RES_STATE;
    }
    return drv_adc.pf_start();
}

static result_t usb_adc_off(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    drv_adc.pf_stop();
    return RES_OK;
}

static result_t usb_spi_on(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (s_ota_on != 0u)
    {
        return RES_STATE;
    }
    return drv_spi.pf_start();
}

static result_t usb_spi_off(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    drv_spi.pf_stop();
    return RES_OK;
}

static result_t usb_i2c_on(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (s_ota_on != 0u)
    {
        return RES_STATE;
    }
    return drv_i2c.pf_start();
}

static result_t usb_i2c_off(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    drv_i2c.pf_stop();
    return RES_OK;
}

static result_t usb_pwm_on(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (s_ota_on != 0u)
    {
        return RES_STATE;
    }
    rc = drv_pwm_out.pf_start();
    if (rc != RES_OK)
    {
        return rc;
    }
    return drv_pwm_in.pf_start();
}

static result_t usb_pwm_off(const uint8_t *arg, uint16_t arg_len)
{
    result_t rc;

    rc = usb_sw_len(arg, arg_len);
    if (rc != RES_OK)
    {
        return rc;
    }
    drv_pwm_out.pf_stop();
    drv_pwm_in.pf_stop();
    return RES_OK;
}

#if (OTA_FLAG_DBG != 0u)
static const usb_ctrl_row_t s_ctrl[] =
{
    { FRAME_MOD_SYS,  FRAME_OP_FLAG_RD,  usb_flag_rd  },
    { FRAME_MOD_SYS,  FRAME_OP_FLAG_WR,  usb_flag_wr  },
    { FRAME_MOD_SYS,  FRAME_OP_FLAG_CLR, usb_flag_clr },
    { FRAME_MOD_UART, FRAME_OP_ON,       usb_uart_on  },
    { FRAME_MOD_UART, FRAME_OP_OFF,      usb_uart_off },
    { FRAME_MOD_ADC,  FRAME_OP_ON,       usb_adc_on   },
    { FRAME_MOD_ADC,  FRAME_OP_OFF,      usb_adc_off  },
    { FRAME_MOD_SPI,  FRAME_OP_ON,       usb_spi_on   },
    { FRAME_MOD_SPI,  FRAME_OP_OFF,      usb_spi_off  },
    { FRAME_MOD_I2C,  FRAME_OP_ON,       usb_i2c_on   },
    { FRAME_MOD_I2C,  FRAME_OP_OFF,      usb_i2c_off  },
    { FRAME_MOD_PWM,  FRAME_OP_ON,       usb_pwm_on   },
    { FRAME_MOD_PWM,  FRAME_OP_OFF,      usb_pwm_off  },
};
#else
static const usb_ctrl_row_t s_ctrl[] =
{
    { FRAME_MOD_UART, FRAME_OP_ON,  usb_uart_on  },
    { FRAME_MOD_UART, FRAME_OP_OFF, usb_uart_off },
    { FRAME_MOD_ADC,  FRAME_OP_ON,  usb_adc_on   },
    { FRAME_MOD_ADC,  FRAME_OP_OFF, usb_adc_off  },
    { FRAME_MOD_SPI,  FRAME_OP_ON,  usb_spi_on   },
    { FRAME_MOD_SPI,  FRAME_OP_OFF, usb_spi_off  },
    { FRAME_MOD_I2C,  FRAME_OP_ON,  usb_i2c_on   },
    { FRAME_MOD_I2C,  FRAME_OP_OFF, usb_i2c_off  },
    { FRAME_MOD_PWM,  FRAME_OP_ON,  usb_pwm_on   },
    { FRAME_MOD_PWM,  FRAME_OP_OFF, usb_pwm_off  },
};
#endif

static result_t usb_on_ctrl(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint8_t i;

    (void)cmd;
    if (data == NULL || len < 2u)
    {
        return RES_PARAM;
    }

    for (i = 0u; i < (uint8_t)(sizeof(s_ctrl) / sizeof(s_ctrl[0])); i++)
    {
        if (s_ctrl[i].pf_on == NULL)
        {
            continue;
        }
        if (s_ctrl[i].mod == data[0] && s_ctrl[i].op == data[1])
        {
            return s_ctrl[i].pf_on(&data[2], (uint16_t)(len - 2u));
        }
    }
    return RES_STATE;
}

static uint32_t s_expect;
static uint32_t s_page;
static uint16_t s_fill;
static uint32_t s_buf[MEM_FLASH_PAGE / 4u];

/* 与工具\加密工具\密钥.txt 相同。AES-128-CBC，每一面都从这一个向量重新开始。 */
static const uint8_t s_ota_key[16] =
{
    0x2Bu, 0x7Eu, 0x15u, 0x16u, 0x28u, 0xAEu, 0xD2u, 0xA6u,
    0xABu, 0xF7u, 0x15u, 0x88u, 0x09u, 0xCFu, 0x4Fu, 0x3Cu
};
static const uint8_t s_ota_iv0[16] =
{
    0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0Au, 0x0Bu, 0x0Cu, 0x0Du, 0x0Eu, 0x0Fu
};
static uint8_t s_iv[16];

static void ota_copy(uint8_t *dst, const uint8_t *src, uint16_t n)
{
    uint16_t i;

    for (i = 0u; i < n; i++)
    {
        dst[i] = src[i];
    }
}

static void ota_iv_reset(void)
{
    ota_copy(s_iv, s_ota_iv0, 16u);
}

/* 解开到 dst。调用方在这页写成功之后才把 iv 存回去，失败可以重发同一包。 */
static void ota_crypt(uint8_t *dst, const uint8_t *src, uint16_t n, uint8_t *iv)
{
    uint16_t off;

    for (off = 0u; off < n; off = (uint16_t)(off + 16u))
    {
        ota_copy(&dst[off], &src[off], 16u);
        aes_sw_driver.pf_aes128_decrypt(iv, &dst[off], s_ota_key);
    }
}

static uint32_t ota_face_base(uint8_t slot)
{
    return MEM_FLASH_PHY_BASE + ((slot == 0u) ? MEM_SLOT_A_OFS : MEM_SLOT_B_OFS);
}

static result_t usb_on_ota_info(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    ota_flag_t rec;
    uint8_t body[13];
    uint32_t ver;
    uint8_t i;

    (void)cmd;
    (void)data;
    if (len != 0u)
    {
        return RES_PARAM;
    }
    for (i = 0u; i < (uint8_t)sizeof(body); i++)
    {
        body[i] = 0u;
    }
    /* 当前面是正在执行的这一面。记录里的 run_slot 要复位后才生效。 */
    body[0] = (uint8_t)APP_SLOT;
    ver = FW_VERSION;
    body[1] = (uint8_t)ver;
    body[2] = (uint8_t)(ver >> 8);
    body[3] = (uint8_t)(ver >> 16);
    body[4] = (uint8_t)(ver >> 24);
    body[7] = OTA_SLOT_NONE;
    if (ota_boot.pf_load(&rec) == RES_OK)
    {
        body[5] = rec.slot[OTA_SLOT_A].state;
        body[6] = rec.slot[OTA_SLOT_B].state;
        body[7] = rec.trial_slot;
        body[8] = rec.trial_cnt;
    }
    if (s_ota_on != 0u)
    {
        body[9] = (uint8_t)s_expect;
        body[10] = (uint8_t)(s_expect >> 8);
        body[11] = (uint8_t)(s_expect >> 16);
        body[12] = (uint8_t)(s_expect >> 24);
    }
    return app_usb_comm_report(FRAME_CMD_OTA_INFO_RSP, body, (uint16_t)sizeof(body));
}

static uint32_t usb_u32(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/* 目标面是当前没在跑的那一面。只记大小和 CRC，并把该面标成正在写。 */
static result_t usb_on_ota_start(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    ota_flag_t rec;
    const uint8_t *field;
    uint8_t target;
    uint8_t i;

    (void)cmd;
    if ((data == 0) || (len != 16u))
    {
        return RES_PARAM;
    }
    target = (uint8_t)(APP_SLOT ^ 1u);
    field = &data[(uint16_t)target * 8u];
    if (usb_u32(field) != MEM_SLOT_SIZE)
    {
        return RES_PARAM;
    }
    for (i = 0u; i < (uint8_t)(sizeof(rec) / 4u); i++)
    {
        ((uint32_t *)&rec)[i] = 0u;
    }
    (void)ota_boot.pf_load(&rec);
    rec.run_slot = (uint8_t)APP_SLOT;
    rec.trial_slot = OTA_SLOT_NONE;
    rec.trial_cnt = 0u;
    rec.slot[target].state = OTA_ST_WRITING;
    rec.slot[target].size = usb_u32(field);
    rec.slot[target].crc32 = usb_u32(field + 4u);
    if (ota_boot.pf_save(&rec) != RES_OK)
    {
        return RES_STATE;
    }
    s_ota_on = 1u;
    usb_biz_halt();
    s_expect = ota_face_base(target);
    s_page = s_expect;
    s_fill = 0u;
    ota_iv_reset();
    return RES_OK;
}

static uint8_t ota_cover(uint32_t addr, uint16_t n, uint32_t base)
{
    if ((n == 0u) || (addr < base) || (n > MEM_SLOT_SIZE))
    {
        return 0u;
    }
    return (uint8_t)((addr - base) <= (MEM_SLOT_SIZE - n));
}

/* 失败时不改 s_fill / s_page，调用方按原期望地址重发。 */
static result_t ota_take(const uint8_t *p, uint16_t n)
{
    uint8_t *raw;
    uint16_t fill;
    uint16_t room;
    uint16_t k;
    uint16_t i;
    uint32_t page;
    result_t rc;

    raw = (uint8_t *)s_buf;
    fill = s_fill;
    page = s_page;
    while (n != 0u)
    {
        room = (uint16_t)(MEM_FLASH_PAGE - fill);
        k = (n < room) ? n : room;
        for (i = 0u; i < k; i++)
        {
            raw[fill + i] = p[i];
        }
        fill = (uint16_t)(fill + k);
        p += k;
        n = (uint16_t)(n - k);
        if (fill != MEM_FLASH_PAGE)
        {
            continue;
        }
        rc = bsp_flash.pf_erase(page, MEM_FLASH_PAGE);
        if (rc != RES_OK)
        {
            return rc;
        }
        rc = bsp_flash.pf_write_page(page, s_buf);
        if (rc != RES_OK)
        {
            return rc;
        }
        page += MEM_FLASH_PAGE;
        fill = 0u;
    }
    s_fill = fill;
    s_page = page;
    return RES_OK;
}

static result_t ota_nack(uint8_t cmd, result_t rc)
{
    uint8_t body[6];

    body[0] = cmd;
    body[1] = (uint8_t)rc;
    body[2] = (uint8_t)s_expect;
    body[3] = (uint8_t)(s_expect >> 8);
    body[4] = (uint8_t)(s_expect >> 16);
    body[5] = (uint8_t)(s_expect >> 24);
    (void)app_usb_comm_report(FRAME_CMD_NACK, body, 6u);
    return RES_OK;
}

/* 正在跑的那一面直接应答。另一面必须按期望地址顺序来，满 256 字节写一页。 */
static result_t usb_on_ota_data(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint8_t plain[FRAME_OTA_DATA_MAX];
    uint8_t iv[16];
    uint32_t addr;
    uint32_t base;
    uint16_t n;
    result_t rc;

    if (s_ota_on == 0u)
    {
        return RES_STATE;
    }
    if ((data == 0) || (len <= 4u) || ((len - 4u) > FRAME_OTA_DATA_MAX))
    {
        return RES_PARAM;
    }
    addr = usb_u32(data);
    n = (uint16_t)(len - 4u);
    if (ota_cover(addr, n, ota_face_base((uint8_t)APP_SLOT)) != 0u)
    {
        usb_reply(FRAME_CMD_ACK, cmd, RES_OK);
        return RES_OK;
    }
    base = ota_face_base((uint8_t)(APP_SLOT ^ 1u));
    if (ota_cover(addr, n, base) == 0u)
    {
        return ota_nack(cmd, RES_PARAM);
    }
    if ((addr + n) <= s_expect)
    {
        usb_reply(FRAME_CMD_ACK, cmd, RES_OK);
        return RES_OK;
    }
    if (addr != s_expect)
    {
        return ota_nack(cmd, RES_PARAM);
    }
    if ((n & 15u) != 0u)
    {
        return ota_nack(cmd, RES_PARAM);
    }
    ota_copy(iv, s_iv, 16u);
    ota_crypt(plain, &data[4], n, iv);
    rc = ota_take(plain, n);
    if (rc != RES_OK)
    {
        return ota_nack(cmd, rc);
    }
    ota_copy(s_iv, iv, 16u);
    s_expect += n;
    usb_reply(FRAME_CMD_ACK, cmd, RES_OK);
    return RES_OK;
}

/* 应答发出去之后再断上拉、复位。复位进 Boot，由 Boot 跳到待试运行的那一面。 */
static void ota_reboot(void)
{
    uint32_t t0;

    t0 = bsp_time.pf_now_ms();
    while ((drv_usb.pf_tx_ready() == 0u) &&
           ((uint32_t)(bsp_time.pf_now_ms() - t0) < 50u))
    {
    }
    drv_usb_detach();
    t0 = bsp_time.pf_now_ms();
    while ((uint32_t)(bsp_time.pf_now_ms() - t0) < 50u)
    {
    }
    NVIC_SystemReset();
}

static result_t usb_on_ota_end(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    ota_flag_t rec;
    volatile img_hdr_t *hdr;
    uint32_t base;
    uint8_t target;

    (void)data;
    if (len != 0u)
    {
        return RES_PARAM;
    }
    if (ota_boot.pf_load(&rec) != RES_OK)
    {
        return RES_STATE;
    }
    target = (uint8_t)(APP_SLOT ^ 1u);
    base = ota_face_base(target);
    if ((rec.slot[target].state != OTA_ST_WRITING) ||
        (rec.slot[target].size != MEM_SLOT_SIZE))
    {
        return RES_STATE;
    }
    if ((s_ota_on != 0u) &&
        ((s_fill != 0u) || (s_expect != (base + MEM_SLOT_SIZE))))
    {
        return RES_STATE;
    }
    if (crc_sw_driver.pf_crc32((const uint8_t *)base, MEM_SLOT_SIZE) != rec.slot[target].crc32)
    {
        return RES_STATE;
    }
    hdr = (volatile img_hdr_t *)(base + MEM_IMG_HDR_OFS);
    if ((hdr->magic != MEM_IMG_MAGIC) || (hdr->link_ofs != (base - MEM_FLASH_PHY_BASE)))
    {
        return RES_STATE;
    }
    rec.run_slot = (uint8_t)APP_SLOT;
    rec.trial_slot = target;
    rec.trial_cnt = 0u;
    rec.slot[target].state = OTA_ST_READY;
    if (ota_boot.pf_save(&rec) != RES_OK)
    {
        return RES_STATE;
    }
    s_ota_on = 0u;
    usb_reply(FRAME_CMD_ACK, cmd, RES_OK);
    ota_reboot();
    return RES_OK;
}

/* 序号 0 读最新一条。设备只回 0x62，不再另回 ACK。 */
static result_t usb_on_log(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint8_t body[LOG_REC_BYTES];
    result_t rc;

    (void)cmd;
    if ((data == 0) || (len != 4u))
    {
        return RES_PARAM;
    }
    rc = log_read(usb_u32(data), body);
    if (rc != RES_OK)
    {
        return rc;
    }
    return app_usb_comm_report(FRAME_CMD_LOG_RSP, body, LOG_REC_BYTES);
}

/* 模式不在 0～3，或动作不是「只改模式 / 发送」，由 app_spi_cfg 回参数不对。没打开回状态不对。 */
static result_t usb_on_spi(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    (void)cmd;
    return app_spi_cfg(data, len);
}

static const usb_cmd_row_t s_cmd[] =
{
    { FRAME_CMD_PING,      1u, usb_on_ping      },
    { FRAME_CMD_CTRL,      1u, usb_on_ctrl      },
    { FRAME_CMD_UART_TX,   0u, usb_on_uart_tx   },
    { FRAME_CMD_UART_LINE, 1u, usb_on_uart_line },
    { FRAME_CMD_SPI_CFG,   1u, usb_on_spi       },
    { FRAME_CMD_OTA_INFO,  0u, usb_on_ota_info  },
    { FRAME_CMD_OTA_START, 1u, usb_on_ota_start },
    { FRAME_CMD_OTA_DATA,  0u, usb_on_ota_data  },
    { FRAME_CMD_OTA_END,   0u, usb_on_ota_end   },
    { FRAME_CMD_LOG,       0u, usb_on_log       },
};

static void usb_dispatch(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint8_t i;
    result_t rc;

    for (i = 0u; i < (uint8_t)(sizeof(s_cmd) / sizeof(s_cmd[0])); i++)
    {
        if (s_cmd[i].cmd != cmd)
        {
            continue;
        }
        rc = s_cmd[i].pf_on(cmd, data, len);
        if (s_cmd[i].ack != 0u)
        {
            usb_reply((rc == RES_OK) ? FRAME_CMD_ACK : FRAME_CMD_NACK, cmd, rc);
        }
        else if (rc != RES_OK)
        {
            usb_reply(FRAME_CMD_NACK, cmd, rc);
        }
        return;
    }
    usb_reply(FRAME_CMD_NACK, cmd, RES_STATE);
}

static void usb_down(void)
{
    uint8_t pkt[FRAME_MAX];
    uint8_t n;
    uint16_t flen;
    uint16_t got;
    uint8_t cmd;
    const uint8_t *data;
    uint16_t data_len;

    n = 0u;
    while (n < USB_DOWN_FRAME_MAX)
    {
        if (!rb_sw_driver.pf_rb_sync_and_peek(&s_usb_rx, FRAME_SOF, FRAME_MIN, FRAME_MAX, &flen))
        {
            return;
        }
        got = rb_sw_driver.pf_rb_read_buf(&s_usb_rx, pkt, flen);
        n++;
        if (got != flen)
        {
            return;
        }
        if (frame_sw_driver.pf_unpack(pkt, flen, &cmd, &data, &data_len) != RES_OK)
        {
            continue;
        }
        usb_dispatch(cmd, data, data_len);
    }
}

/*
 * 环空不发。USB 发不出也不读环。
 * 环里的字节少于本帧数据上限，有多少带多少。
 * 多于上限，只带走上限，剩下下次再来。
 */
static void usb_up_uart(void)
{
    rb_t *rb;
    uint8_t raw[FRAME_DATA_MAX];
    uint8_t pkt[FRAME_MAX];
    uint8_t n;
    uint16_t room;
    uint16_t avail;
    uint16_t take;
    uint16_t packed;
    uint16_t budget;

    if (s_uart_report == 0u)
    {
        return;
    }
    rb = drv_uart.pf_rx_ring();
    if (rb == NULL)
    {
        return;
    }

    n = 0u;
    budget = UART_RX_TAKE_MAX;
    while (n < USB_UP_FRAME_MAX && budget > 0u)
    {
        room = drv_usb.pf_tx_ready();
        if (room < FRAME_MIN)
        {
            return;
        }

        avail = rb_sw_driver.pf_rb_available(rb);
        if (avail == 0u)
        {
            return;
        }

        take = FRAME_DATA_MAX;
        if (take > (uint16_t)(room - FRAME_OH))
        {
            take = (uint16_t)(room - FRAME_OH);
        }
        if (take > avail)
        {
            take = avail;
        }
        if (take > budget)
        {
            take = budget;
        }
        if (take == 0u)
        {
            return;
        }

        if (rb_sw_driver.pf_rb_read_buf(rb, raw, take) != take)
        {
            return;
        }
        if (frame_sw_driver.pf_pack(FRAME_CMD_UART_RX, raw, take, pkt, (uint16_t)sizeof(pkt), &packed) != RES_OK)
        {
            return;
        }
        if (drv_usb.pf_tx(pkt, packed) != RES_OK)
        {
            return;
        }
        budget = (uint16_t)(budget - take);
        n++;
    }
}

/* 一拍最多两帧，给串口留出发送余量。40 字节能进一帧，多出来的下次再报。 */
static void usb_up_spi(void)
{
    rb_t *rb;
    uint8_t raw[FRAME_DATA_MAX];
    uint8_t pkt[FRAME_MAX];
    uint8_t n;
    uint16_t room;
    uint16_t avail;
    uint16_t take;
    uint16_t packed;

    rb = drv_spi.pf_rx_ring();
    if (rb == NULL)
    {
        return;
    }

    n = 0u;
    while (n < 2u)
    {
        room = drv_usb.pf_tx_ready();
        if (room < FRAME_MIN)
        {
            return;
        }

        avail = rb_sw_driver.pf_rb_available(rb);
        if (avail == 0u)
        {
            return;
        }

        take = FRAME_DATA_MAX;
        if (take > (uint16_t)(room - FRAME_OH))
        {
            take = (uint16_t)(room - FRAME_OH);
        }
        if (take > avail)
        {
            take = avail;
        }
        if (take == 0u)
        {
            return;
        }

        if (rb_sw_driver.pf_rb_read_buf(rb, raw, take) != take)
        {
            return;
        }
        if (frame_sw_driver.pf_pack(FRAME_CMD_SPI, raw, take, pkt, (uint16_t)sizeof(pkt), &packed) != RES_OK ||
            drv_usb.pf_tx(pkt, packed) != RES_OK)
        {
            if (rb_sw_driver.pf_rb_available(rb) == 0u)
            {
                (void)rb_sw_driver.pf_rb_write_buf(rb, raw, take);
            }
            return;
        }
        n++;
    }
}

result_t app_usb_comm_init(void)
{
#if (USB_LOG_ENABLE != 0u) && (PRINT_ENABLE != 0u)
    usb_log_bind(print_sw_driver.pf_printf);
#endif
    rb_sw_driver.pf_rb_init(&s_usb_rx);
    (void)drv_usb.pf_on_rx_set(usb_on_rx);
    return drv_usb.pf_init();
}

result_t app_usb_comm_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event)
{
    (void)now_ms;
    (void)elapsed_ms;
    (void)event;

    usb_down();
    usb_up_uart();
    usb_up_spi();
    return RES_OK;
}
