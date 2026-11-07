/*
 * bsp_clk.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 系统时钟、外设时钟使能封装
 *
 * 对外表名不带芯片型号。原厂 RCC 函数只出现在 bsp_clk.c。
 */
#ifndef FW_APP_02_BSP_CLK_BSP_CLK_H_
#define FW_APP_02_BSP_CLK_BSP_CLK_H_

#include <stdint.h>
#include "bsp_gpio.h"

typedef struct
{
    void     (*pf_init)(void);
    uint32_t (*pf_core_hz)(void);
    void     (*pf_gpio_clock_on)(bsp_pin_id_t id);
} bsp_clk_driver_t;

extern const bsp_clk_driver_t bsp_clk;

#endif /* FW_APP_02_BSP_CLK_BSP_CLK_H_ */
