/*
 * sched_tasks.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 整机任务表。加任务只改 sched_tasks.c 和对应的应用文件。
 */
#ifndef FW_APP_05_SCHEDULER_SCHED_TASKS_SCHED_TASKS_H_
#define FW_APP_05_SCHEDULER_SCHED_TASKS_SCHED_TASKS_H_

#include "sched.h"

void sched_tasks_bind(const sched_task_t **table, sched_task_rt_t **rt, uint8_t *count);

#endif /* FW_APP_05_SCHEDULER_SCHED_TASKS_SCHED_TASKS_H_ */
