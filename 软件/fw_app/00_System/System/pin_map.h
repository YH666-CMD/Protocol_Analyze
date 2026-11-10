/*
 * pin_map.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 所有引脚定义、外设映射（所有引脚只在这里定义）
 *
 * 02_BSP 和 03_Driver 的 .c 可以包含本文件。应用层不包含，也不看 BOARD_HEARTBEAT_ENABLE。
 * bsp_pin_id_t 只收应用直接拨动的脚。串口、SPI、ADC 的脚写在这里，
 * 由驱动使用，不要加进 bsp_pin_id_t。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_
#define FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_

#include "ch32v20x_gpio.h"

/* USART1。发送 PA9，接收 PA10。只给 drv_uart.c 用。 */
#define BOARD_UART_TX_PORT      GPIOA
#define BOARD_UART_TX_PIN       GPIO_Pin_9
#define BOARD_UART_RX_PORT      GPIOA
#define BOARD_UART_RX_PIN       GPIO_Pin_10

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
