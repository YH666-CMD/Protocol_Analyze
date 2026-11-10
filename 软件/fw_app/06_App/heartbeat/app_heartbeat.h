/*
 * app_heartbeat.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 第一个周期任务。计数大约每秒加一。
 *
 * 开关打开后，每次调用翻转一次电平，亮灭一周是 2 秒。
 * g_app_heartbeat_count 留给调试器观察。
 * event 为 0 表示周期到了。心跳不处理事件。
 */
#ifndef FW_APP_06_APP_HEARTBEAT_APP_HEARTBEAT_H_
#define FW_APP_06_APP_HEARTBEAT_APP_HEARTBEAT_H_

#include <stdint.h>
#include "sched.h"

extern volatile uint32_t g_app_heartbeat_count;

result_t app_heartbeat_init(void);
result_t app_heartbeat_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event);

#endif /* FW_APP_06_APP_HEARTBEAT_APP_HEARTBEAT_H_ */
