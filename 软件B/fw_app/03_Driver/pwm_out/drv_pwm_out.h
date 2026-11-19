/*
 * drv_pwm_out.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: TIM1 通道 1 在 PA8 输出 PWM。
 *
 * 约定：
 *   1. pf_start 可以连着调用。只记下已经打开，先不输出。
 *   2. pf_set 把上位机算好的 PSC、ARR、CCR 写进寄存器。单片机不再做除法。
 *   3. pf_stop 可以连着调用。停比较输出，PA8 回到浮空。不用 DMA，不链定时器库。
 *   4. 引脚只从 pin_map.h 取。
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
     * pf_set：写出一档 PWM。没打开返回 RES_STATE。ARR 为 0 返回 RES_PARAM。
     */
    result_t (*pf_set)(uint16_t psc, uint16_t arr, uint16_t ccr);
    /*
     * pf_stop：关掉 PWM 输出。可以连着调用。
     */
    void (*pf_stop)(void);
} drv_pwm_out_driver_t;

extern const drv_pwm_out_driver_t drv_pwm_out;

#endif /* FW_APP_03_DRIVER_PWM_OUT_DRV_PWM_OUT_H_ */
