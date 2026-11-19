/*
 * drv_pwm_in.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: TIM2 通道 1 在 PA15 量 PWM。
 *
 * 约定：
 *   1. pf_start 可以连着调用。打开后开始量，不占中断。
 *   2. 计数是 1 微秒一格。周期超过大约 65 毫秒就当成没有信号。
 *   3. pf_poll 在任务里读结果。和上次相同就不准备上报。
 *   4. pf_stop 可以连着调用。停 TIM2。不用 DMA，不链定时器库。
 *   5. 引脚只从 pin_map.h 取。只关 JTAG，SWD 保留。
 */
#ifndef FW_APP_03_DRIVER_PWM_IN_DRV_PWM_IN_H_
#define FW_APP_03_DRIVER_PWM_IN_DRV_PWM_IN_H_

#include "platform.h"
#include <stdbool.h>

typedef struct
{
    /*
     * pf_start：开始捕获。已经开着就直接返回。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉捕获。可以连着调用。
     */
    void (*pf_stop)(void);
    /*
     * pf_poll：任务里调用。有新的周期，或信号刚消失，才留下一帧。
     */
    void (*pf_poll)(void);
    /*
     * pf_take：拿走 8 字节。前 4 字节小端周期，后 4 字节小端高电平，单位是微秒。
     * 都是 0 表示信号没了。没有新结果就返回 false。
     */
    bool (*pf_take)(uint8_t *dst);
} drv_pwm_in_driver_t;

extern const drv_pwm_in_driver_t drv_pwm_in;

#endif /* FW_APP_03_DRIVER_PWM_IN_DRV_PWM_IN_H_ */
