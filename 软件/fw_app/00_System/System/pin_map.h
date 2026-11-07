/*
 * pin_map.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 所有引脚定义、外设映射（所有引脚只在这里定义）
 *
 * 只有 02_BSP 的 .c 可以包含本文件。应用层不看 BOARD_HEARTBEAT_ENABLE。
 * bsp_pin_id_t 只收应用直接拨动的脚。串口、SPI、ADC 的脚以后写在这里，
 * 由驱动使用，不要加进 bsp_pin_id_t。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_
#define FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_

/* 0：心跳脚未配置，bsp_gpio 的翻转直接返回。 */
#define BOARD_HEARTBEAT_ENABLE  0

#if BOARD_HEARTBEAT_ENABLE
#include "ch32v20x_gpio.h"
/*
 * 打开上面的开关前，改成板上实际空闲的指示灯，例如：
 * #define BOARD_HEARTBEAT_PORT GPIOA
 * #define BOARD_HEARTBEAT_PIN  GPIO_Pin_13
 */
#ifndef BOARD_HEARTBEAT_PORT
#error "BOARD_HEARTBEAT_ENABLE is 1: define BOARD_HEARTBEAT_PORT in pin_map.h"
#endif
#ifndef BOARD_HEARTBEAT_PIN
#error "BOARD_HEARTBEAT_ENABLE is 1: define BOARD_HEARTBEAT_PIN in pin_map.h"
#endif
#endif

#endif /* FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_ */
