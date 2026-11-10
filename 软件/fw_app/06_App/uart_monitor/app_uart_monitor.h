/*
 * app_uart_monitor.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 串口接收环的测试消费者。看到环里有数就拿走一段，用打印函数送出去。
 *
 * 约定：
 *   1. 一次最多拿走 UART_RX_TAKE_MAX 个字节，并且不超过发送队列的空位。剩下的留在环里。
 *   2. 拿走后放进发送队列就返回，不等字节离开芯片。发送和接收同时进行。
 *   3. PRINT_ENABLE 为 0 时本任务不读环。环只能有一个消费者，后期留给 USB。
 *   4. 本任务不解析协议，也不认识 USB 帧。
 */
#ifndef FW_APP_06_APP_UART_MONITOR_APP_UART_MONITOR_H_
#define FW_APP_06_APP_UART_MONITOR_APP_UART_MONITOR_H_

#include <stdint.h>
#include "sched.h"

result_t app_uart_monitor_init(void);
result_t app_uart_monitor_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event);

#endif /* FW_APP_06_APP_UART_MONITOR_APP_UART_MONITOR_H_ */
