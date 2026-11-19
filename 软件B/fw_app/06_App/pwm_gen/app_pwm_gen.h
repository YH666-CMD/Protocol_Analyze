/*
 * app_pwm_gen.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 把 0xA0 的三个寄存器值交给 PWM 输出。
 */
#ifndef FW_APP_06_APP_PWM_GEN_APP_PWM_GEN_H_
#define FW_APP_06_APP_PWM_GEN_APP_PWM_GEN_H_

#include "platform.h"

result_t app_pwm_out(const uint8_t *data, uint16_t len);

#endif /* FW_APP_06_APP_PWM_GEN_APP_PWM_GEN_H_ */
