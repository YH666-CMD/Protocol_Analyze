/*
 * sched.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 调度器核心：时间片管理、任务注册、轮询执行
 *
 * 纯软件。不包含 config.h，不包含板级头文件。
 * next_due / last_run 由调用方提供，调度器只保存指针和个数。
 *
 * 任务函数必须返回。不写 delay_ms，不等待硬件标志。
 * 超时用 on_tick 的 elapsed_ms 累加，不假设每次调用刚好隔一个周期。
 */
#ifndef FW_APP_05_SCHEDULER_SCHED_SCHED_H_
#define FW_APP_05_SCHEDULER_SCHED_SCHED_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*sched_init_fn)(void);/*任务初始化函数*/
/*形参：调度器这一次看时间读到的毫秒值，距离这个任务上次运行过去了多少毫秒*/
typedef void (*sched_tick_fn)(uint32_t now_ms, uint32_t elapsed_ms);/*任务周期函数*/
typedef uint32_t (*sched_now_ms_fn)(void);/*现在几毫秒函数*/
typedef void (*sched_hook_fn)(void);/*无返回值的钩子idle或kick*/

typedef struct
{
    const char   *name;       /*名字*/
    uint16_t      period_ms;  /*周期 0表示没有周期函数只借用初始化*/
    uint16_t      phase_ms;   /*相位（必须比周期小）,把多个任务错开避免同时到来，执行周期为周期+相位，多个1ms周期任务无法错开*/
    sched_init_fn pf_init;    /*初始化*/
    sched_tick_fn pf_on_tick; /*周期调用task*/
} sched_task_t;

typedef struct
{
    uint32_t next_due;/*下一次应该运行的绝对毫秒时刻*/
    uint32_t last_run;/*上一次运行的绝对毫秒时刻*/
} sched_task_rt_t;

typedef struct
{
    /*形参：（任务描述表，运行时数组，表里有几个任务，读毫秒的函数指针，
    毫秒没变时的钩子 NULL表示空闲时什么也不做，处理完到期任务后调用的钩子 看门狗可挂在这）*/
    void (*pf_init)(const sched_task_t *table, sched_task_rt_t *rt, uint8_t count,
                    sched_now_ms_fn now_ms, sched_hook_fn idle, sched_hook_fn kick);
    void (*pf_poll)(void);
} sched_driver_interface_t;

extern const sched_driver_interface_t sched_sw_driver;

#endif /* FW_APP_05_SCHEDULER_SCHED_SCHED_H_ */
