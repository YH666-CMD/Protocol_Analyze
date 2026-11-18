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
#include "frame.h"
#include "ring_buffer.h"
#include "usb_cfg.h"
#include "usb_log.h"
#if (USB_LOG_ENABLE != 0u) && (PRINT_ENABLE != 0u)
#include "print.h"
#endif
#include <stddef.h>
#include "ota_boot.h"
#if (OTA_FLAG_DBG != 0u)
#include "bsp_flash.h"
#endif

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

#if (OTA_FLAG_DBG != 0u)
static const usb_ctrl_row_t s_ctrl[] =
{
    { FRAME_MOD_SYS, FRAME_OP_FLAG_RD,  usb_flag_rd  },
    { FRAME_MOD_SYS, FRAME_OP_FLAG_WR,  usb_flag_wr  },
    { FRAME_MOD_SYS, FRAME_OP_FLAG_CLR, usb_flag_clr },
};
#else
static const usb_ctrl_row_t s_ctrl[] =
{
    { 0u, 0u, NULL },
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
    return ota_boot.pf_save(&rec);
}

static const usb_cmd_row_t s_cmd[] =
{
    { FRAME_CMD_PING,      1u, usb_on_ping      },
    { FRAME_CMD_CTRL,      1u, usb_on_ctrl      },
    { FRAME_CMD_UART_TX,   0u, usb_on_uart_tx   },
    { FRAME_CMD_UART_LINE, 1u, usb_on_uart_line },
    { FRAME_CMD_OTA_INFO,  0u, usb_on_ota_info  },
    { FRAME_CMD_OTA_START, 1u, usb_on_ota_start },
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
    return RES_OK;
}
