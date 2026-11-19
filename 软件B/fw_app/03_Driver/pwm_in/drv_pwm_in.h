/*
 * drv_pwm_in.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: PWM 输入开关。捕获留到后面。
 *
 * 约定：
 *   1. pf_start 可以连着调用。驱动还没写完时只记下已经打开。
 *   2. pf_stop 可以连着调用。停 TIM2 捕获和中断。不用 DMA。
 *   3. PA15 默认还是调试脚。这里不改重映射，避免把调试口关掉。
 */
#ifndef FW_APP_03_DRIVER_PWM_IN_DRV_PWM_IN_H_
#define FW_APP_03_DRIVER_PWM_IN_DRV_PWM_IN_H_

#include "platform.h"

typedef struct
{
    /*
     * pf_start：记下 PWM 输入已经打开。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 PWM 输入。可以连着调用。
     */
    void (*pf_stop)(void);
} drv_pwm_in_driver_t;

extern const drv_pwm_in_driver_t drv_pwm_in;

#endif /* FW_APP_03_DRIVER_PWM_IN_DRV_PWM_IN_H_ */
