#include "app_uart_monitor.h"
#include "config.h"
#include "drv_uart.h"
#if (PRINT_ENABLE != 0u)
#include "print.h"
#include "ring_buffer.h"
#endif
#include <stddef.h>

/*
 * 先看发送队列还空多少，再从环里拿，拿到就入队。
 * 入队后立刻返回。字节由发送中断写出去，这里不等线路。
 */
#if (PRINT_ENABLE != 0u)
static void uart_rx_echo(void)
{
    rb_t *rb;
    uint16_t budget;
    uint8_t byte;

    rb = drv_uart.pf_rx_ring();
    if (rb == NULL)
    {
        return;
    }

    budget = UART_RX_TAKE_MAX;
    while (budget > 0u)
    {
        if (drv_uart.pf_tx_free() == 0u)
        {
            return;
        }
        if (!rb_sw_driver.pf_rb_read(rb, &byte))
        {
            return;
        }
        (void)drv_uart.pf_putc(byte);
        budget--;
    }
}
#endif

result_t app_uart_monitor_init(void)
{
    result_t rc;

    rc = drv_uart.pf_init();
    if (rc != RES_OK)
    {
        return rc;
    }

#if (PRINT_ENABLE != 0u)
    rc = print_sw_driver.pf_init(drv_uart.pf_putc);
    if (rc != RES_OK)
    {
        return rc;
    }
    (void)print_sw_driver.pf_printf("uart rx ready\r\n");
#endif
    return RES_OK;
}

result_t app_uart_monitor_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event)
{
#if (PRINT_ENABLE != 0u)
    (void)now_ms;
    (void)elapsed_ms;
    (void)event;

    uart_rx_echo();
    return RES_OK;
#else
    (void)now_ms;
    (void)elapsed_ms;
    (void)event;
    return RES_OK;
#endif
}
