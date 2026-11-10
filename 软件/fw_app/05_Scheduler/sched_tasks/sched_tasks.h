/*
 * sched_tasks.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 整机任务表。这里只交出表，不再包一层别人的接口。
 */
#ifndef FW_APP_05_SCHEDULER_SCHED_TASKS_SCHED_TASKS_H_
#define FW_APP_05_SCHEDULER_SCHED_TASKS_SCHED_TASKS_H_

#include "sched.h"

/*
 * sched_tasks_bind：把整机任务表交给调用方。
 *
 * 形参：
 *   table —— 输出：指向 const 任务表；不需要可以传 NULL；
 *   rt    —— 输出：指向运行时数组；不需要可以传 NULL；
 *   count —— 输出：任务个数；不需要可以传 NULL。
*/
void sched_tasks_bind(const sched_task_t **table, sched_task_rt_t **rt, uint8_t *count);

#endif /* FW_APP_05_SCHEDULER_SCHED_TASKS_SCHED_TASKS_H_ */
