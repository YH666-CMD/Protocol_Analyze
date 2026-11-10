/*
 * app_usb_comm.c
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 任务。这一版只把设备交给主机枚举。
 *
 * 约定：
 *   1. 初始化里调用 drv_usb.pf_init。USB 日志和串口打印都打开时，先把 printf 登记进去。
 *      串口打印关掉后，日志没有出口，这里不再引用打印函数。串口发送队列仍在。
 *   2. 不读串口接收环，也不把 USB 收到的字节写进环。环的衔接以后再做。
 *   3. on_tick 现在不搬数据。以后在这里看 pf_tx_ready，再从环里取出字节调用 pf_tx。
 */
#include "app_usb_comm.h"
#include "config.h"
#include "drv_usb.h"
#include "usb_cfg.h"
#include "usb_log.h"
#if (USB_LOG_ENABLE != 0u) && (PRINT_ENABLE != 0u)
#include "print.h"
#endif

result_t app_usb_comm_init(void)
{
#if (USB_LOG_ENABLE != 0u) && (PRINT_ENABLE != 0u)
    usb_log_bind(print_sw_driver.pf_printf);
#endif
    return drv_usb.pf_init();
}

result_t app_usb_comm_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event)
{
    (void)now_ms;
    (void)elapsed_ms;
    (void)event;
    return RES_OK;
}
