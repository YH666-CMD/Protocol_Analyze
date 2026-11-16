/*
 * bsp_gpio.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: GPIO初始化、引脚读写封装
 *
 * 枚举只收应用逻辑直接拨动的脚。外设脚留在 pin_map.h，不进这个枚举。
 * 脚未配置时，翻转、写、读直接返回。应用层不判断开关宏。
 */
#ifndef FW_APP_02_BSP_GPIO_BSP_GPIO_H_
#define FW_APP_02_BSP_GPIO_BSP_GPIO_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    BSP_PIN_HEARTBEAT = 0,
    BSP_PIN_COUNT
} bsp_pin_id_t;

typedef struct
{
    void (*pf_init)(void);
    void (*pf_write)(bsp_pin_id_t id, bool level_high);
    void (*pf_toggle)(bsp_pin_id_t id);
    bool (*pf_read)(bsp_pin_id_t id);
} bsp_gpio_driver_t;

extern const bsp_gpio_driver_t bsp_gpio;

#endif /* FW_APP_02_BSP_GPIO_BSP_GPIO_H_ */
