/*
 * app_uart_monitor.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 串口驱动初始化。接收环的消费者是 USB 协议任务。
 *
 * 约定：
 *   1. 本任务只打开 USART1。PRINT_ENABLE 为 1 时再登记打印出口，给 USB 日志用。
 *   2. 不读接收环。环只能有一个消费者，已经交给 USB。
 *   3. 本任务不解析协议，也不认识 USB 帧。
 */
#ifndef FW_APP_06_APP_UART_MONITOR_APP_UART_MONITOR_H_
#define FW_APP_06_APP_UART_MONITOR_APP_UART_MONITOR_H_

#include <stdint.h>
#include "sched.h"

result_t app_uart_monitor_init(void);
result_t app_uart_monitor_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event);

#endif /* FW_APP_06_APP_UART_MONITOR_APP_UART_MONITOR_H_ */
