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

/*
 * 还没有具体动作。占位行的处理函数是空，不会被调用。
 * 以后在表里加 { 模块, 动作, 处理函数 } 即可。
 */
static const usb_ctrl_row_t s_ctrl[] =
{
    { 0u, 0u, NULL },
};

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

static const usb_cmd_row_t s_cmd[] =
{
    { FRAME_CMD_PING,      1u, usb_on_ping      },
    { FRAME_CMD_CTRL,      1u, usb_on_ctrl      },
    { FRAME_CMD_UART_TX,   0u, usb_on_uart_tx   },
    { FRAME_CMD_UART_LINE, 1u, usb_on_uart_line },
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
