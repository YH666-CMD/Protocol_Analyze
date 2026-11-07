/*
 * config.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 总配置开关（使能模块、功能开关）
 *
 * 调度器不包含本文件。任务周期由 sched_tasks.c 使用。
 * 丢拍阈值写在 sched.c 内部。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_CONFIG_H_
#define FW_APP_00_SYSTEM_SYSTEM_CONFIG_H_

#define SCHED_TICK_HZ                1000u
#define SCHED_TASK_MAX               8u
#define SCHED_PERIOD_HEARTBEAT_MS    1000u
#define SCHED_PHASE_HEARTBEAT_MS     0u

#endif /* FW_APP_00_SYSTEM_SYSTEM_CONFIG_H_ */
