/*
 * app_usb_comm.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 枚举任务。这一版只把设备交给主机认成虚拟串口。
 *
 * 约定：
 *   1. 初始化里调用 drv_usb.pf_init。打印开关打开时，先把本工程的 printf 登记进去。
 *   2. 不读串口接收环，也不把 USB 收到的字节写进环。环的衔接以后再做。
 *   3. on_tick 现在不搬数据。以后在这里看 pf_tx_ready，再从环里取出字节调用 pf_tx。
 */
#ifndef FW_APP_06_APP_USB_COMM_APP_USB_COMM_H_
#define FW_APP_06_APP_USB_COMM_APP_USB_COMM_H_

#include <stdint.h>
#include "sched.h"

result_t app_usb_comm_init(void);
result_t app_usb_comm_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event);

#endif /* FW_APP_06_APP_USB_COMM_APP_USB_COMM_H_ */
