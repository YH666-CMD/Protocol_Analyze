/*
 * pin_map.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 所有引脚定义、外设映射（所有引脚只在这里定义）
 *
 * 02_BSP 和 03_Driver 的 .c 可以包含本文件。应用层不包含，也不看 BOARD_HEARTBEAT_ENABLE。
 * bsp_pin_id_t 只收应用直接拨动的脚。串口、ADC、SPI、I2C、PWM、USB 的脚写在这里，
 * 由驱动使用，不要加进 bsp_pin_id_t。应用层不包含本文件。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_
#define FW_APP_00_SYSTEM_SYSTEM_PIN_MAP_H_

#include "ch32v20x_gpio.h"

/* USART1。发送 PA9，接收 PA10。只给 drv_uart.c 用。 */
#define BOARD_UART_TX_PORT      GPIOA
#define BOARD_UART_TX_PIN       GPIO_Pin_9
#define BOARD_UART_RX_PORT      GPIOA
#define BOARD_UART_RX_PIN       GPIO_Pin_10

/* USB。D- 是 PA11，D+ 是 PA12。寄存器写法在 usb_cfg.h，脚不要换。 */
#define BOARD_USB_DM_PORT       GPIOA
#define BOARD_USB_DM_PIN        GPIO_Pin_11
#define BOARD_USB_DP_PORT       GPIOA
#define BOARD_USB_DP_PIN        GPIO_Pin_12

/* ADC 三路。PA0、PA1、PA2。只给 drv_adc.c 用。 */
#define BOARD_ADC_PORT          GPIOA
#define BOARD_ADC_CH0_PIN       GPIO_Pin_0
#define BOARD_ADC_CH1_PIN       GPIO_Pin_1
#define BOARD_ADC_CH2_PIN       GPIO_Pin_2

/* SPI。NSS PA4，SCK PA5，MISO PA6，MOSI PA7。只给 drv_spi.c 用。 */
#define BOARD_SPI_NSS_PORT      GPIOA
#define BOARD_SPI_NSS_PIN       GPIO_Pin_4
#define BOARD_SPI_SCK_PORT      GPIOA
#define BOARD_SPI_SCK_PIN       GPIO_Pin_5
#define BOARD_SPI_MISO_PORT     GPIOA
#define BOARD_SPI_MISO_PIN      GPIO_Pin_6
#define BOARD_SPI_MOSI_PORT     GPIOA
#define BOARD_SPI_MOSI_PIN      GPIO_Pin_7

/* I2C。SCL PB6，SDA PB7，通知 PB4。只给 drv_i2c.c 用。 */
#define BOARD_I2C_SCL_PORT      GPIOB
#define BOARD_I2C_SCL_PIN       GPIO_Pin_6
#define BOARD_I2C_SDA_PORT      GPIOB
#define BOARD_I2C_SDA_PIN       GPIO_Pin_7
#define BOARD_I2C_NOTIFY_PORT   GPIOB
#define BOARD_I2C_NOTIFY_PIN    GPIO_Pin_4

/* PWM。输出 PA8，输入 PA15。分别给 drv_pwm_out.c、drv_pwm_in.c 用。 */
#define BOARD_PWM_OUT_PORT      GPIOA
#define BOARD_PWM_OUT_PIN       GPIO_Pin_8
#define BOARD_PWM_IN_PORT       GPIOA
#define BOARD_PWM_IN_PIN        GPIO_Pin_15

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
