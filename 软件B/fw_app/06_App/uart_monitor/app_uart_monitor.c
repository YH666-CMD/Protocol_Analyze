#include "app_uart_monitor.h"
#include "config.h"
#include "drv_uart.h"
#if (PRINT_ENABLE != 0u)
#include "print.h"
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
#endif
    return RES_OK;
}

result_t app_uart_monitor_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event)
{
    (void)now_ms;
    (void)elapsed_ms;
    (void)event;
    return RES_OK;
}
