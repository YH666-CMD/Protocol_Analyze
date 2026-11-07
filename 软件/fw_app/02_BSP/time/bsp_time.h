/*
 * bsp_time.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 1 ms 时基。pf_init 只配置，pf_start 才开始计数。
 */
#ifndef FW_APP_02_BSP_TIME_BSP_TIME_H_
#define FW_APP_02_BSP_TIME_BSP_TIME_H_

#include <stdint.h>

typedef struct
{
    void     (*pf_init)(void);
    void     (*pf_start)(void);
    uint32_t (*pf_now_ms)(void);
} bsp_time_driver_t;

extern const bsp_time_driver_t bsp_time;

#endif /* FW_APP_02_BSP_TIME_BSP_TIME_H_ */
