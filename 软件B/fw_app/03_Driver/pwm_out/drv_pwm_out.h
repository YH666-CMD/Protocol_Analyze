/*
 * drv_pwm_out.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: PWM 输出开关。频率和占空比留到后面。
 *
 * 约定：
 *   1. pf_start 可以连着调用。驱动还没写完时只记下已经打开。
 *   2. pf_stop 可以连着调用。停 TIM1 比较输出，PA8 回到无输出。不用 DMA。
 *   3. 引脚只从 pin_map.h 取。
 */
#ifndef FW_APP_03_DRIVER_PWM_OUT_DRV_PWM_OUT_H_
#define FW_APP_03_DRIVER_PWM_OUT_DRV_PWM_OUT_H_

#include "platform.h"

typedef struct
{
    /*
     * pf_start：记下 PWM 输出已经打开。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 PWM 输出。可以连着调用。
     */
    void (*pf_stop)(void);
} drv_pwm_out_driver_t;

extern const drv_pwm_out_driver_t drv_pwm_out;

#endif /* FW_APP_03_DRIVER_PWM_OUT_DRV_PWM_OUT_H_ */
