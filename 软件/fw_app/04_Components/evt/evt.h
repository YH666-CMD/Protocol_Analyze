/*
 * evt.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 一个任务把事件发给另一个任务。
 *
 * 约定：
 *   1. pf_send 只把事件放进目标任务的邮箱。发送方接着就要返回。
 *   2. 调度器每从一个任务返回，就 pf_take。有事件就立刻跑那个任务，不等它的周期。
 *   3. 事件码 0 留给“这次是周期到了”。pf_send 传 0 会返回参数错误。
 *   4. 一个任务同时只保留一个没处理的事件。邮箱还满着就返回 RES_NO_MEM。
 *   5. 字节流和采样不要走这里，避免把别的任务饿死。紧急情况才发事件。
 */
#ifndef FW_APP_04_COMPONENTS_EVT_EVT_H_
#define FW_APP_04_COMPONENTS_EVT_EVT_H_

#include <stdint.h>
#include <stdbool.h>
#include "platform.h"

typedef uint32_t (*evt_lock_fn)(void);
typedef void     (*evt_unlock_fn)(uint32_t state);

typedef struct
{
    /*
    * pf_init：初始化邮箱。
    * 形参：
    *   task_count —— 任务个数，必须 1..SCHED_TASK_MAX；
    *   lock       —— 关中断函数；
    *   unlock     —— 恢复中断函数。
    * 后两个必须同时给，或同时为 NULL；只给一个返回 RES_PARAM。
    *
    * 返回：
    *   RES_OK       初始化成功；
    *   RES_PARAM    任务数为 0/超上限，或锁函数只给了一个。
    */
    result_t (*pf_init)(uint8_t task_count, evt_lock_fn lock, evt_unlock_fn unlock);
    /*
     * pf_send：把事件放进目标任务的邮箱。
     * 形参：
     *   task_id —— 目标任务编号（任务表下标）；
     *   event   —— 事件码，不能为 0（0 是“周期到了”的保留值）。
     *
     * 返回：
     *   RES_OK      放进邮箱；
     *   RES_PARAM   任务编号越界，或事件码为 0；
     *   RES_NO_MEM  目标邮箱已有未处理事件，或排队数组满。
    */
    result_t (*pf_send)(uint8_t task_id, uint32_t event);
    /*
    * pf_init：初始化邮箱。
     * pf_take：按发送顺序取出一个事件。
     * 形参：
     *   task_id —— 输出：这个事件属于哪个任务；
     *   event   —— 输出：事件码。
     * 返回：
     *   true  —— 取到了一个事件；
     *   false —— 当前没有事件，或输出指针为空。
    */
    bool     (*pf_take)(uint8_t *task_id, uint32_t *event);
} evt_driver_t;

extern const evt_driver_t evt_sw_driver;

#endif /* FW_APP_04_COMPONENTS_EVT_EVT_H_ */
