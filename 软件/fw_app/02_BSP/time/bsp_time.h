/*
 * bsp_time.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 1 ms 时基。pf_init 只配置，pf_start 才开始计数。
 *
 * pf_wait 在毫秒没变时被调度器调用。函数体是空的，不睡眠。
 * pf_set_ms_hook 把每一毫秒要做的检查交给上层。时基本身不认识调度器。
 */
#ifndef FW_APP_02_BSP_TIME_BSP_TIME_H_
#define FW_APP_02_BSP_TIME_BSP_TIME_H_

#include <stdint.h>
#include "platform.h"

typedef void (*bsp_time_ms_fn)(uint32_t now_ms);

typedef struct
{
    void     (*pf_init)(void);
    void     (*pf_start)(void);
    uint32_t (*pf_now_ms)(void);
    uint32_t (*pf_cycles)(void);   /* 当前这一毫秒内的计数 */
    uint32_t (*pf_now_us)(void);   /* 毫秒乘 1000，加上本毫秒已经走过的微秒 */
    /* 形参：任务名，这次调用花掉的微秒。打印口还没有，函数体先空着 */
    result_t (*pf_print_task)(const char *name, uint32_t cost_us);
    /* 无参。空实现，立刻返回。 */
    void     (*pf_wait)(void);
    /* 形参：每一毫秒调用的函数。传 NULL 表示这一毫秒不再额外做事 */
    void     (*pf_set_ms_hook)(bsp_time_ms_fn hook);
} bsp_time_driver_t;

extern const bsp_time_driver_t bsp_time;

#endif /* FW_APP_02_BSP_TIME_BSP_TIME_H_ */
