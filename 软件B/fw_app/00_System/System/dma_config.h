/*
 * dma_config.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: DMA通道分配、优先级、缓冲区大小配置
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_DMA_CONFIG_H_
#define FW_APP_00_SYSTEM_SYSTEM_DMA_CONFIG_H_

/*
 * 通道号按 CH32V203 的默认请求写死，驱动里再换成库的通道指针。
 * 这里不包含外设头文件，避免配置头把芯片类型泄漏出去。
 *
 *   5  USART1 接收   DMA1 通道 5    现在使用
 *   4  USART1 发送   DMA1 通道 4    测试打印走轮询，不用这条
 *   1  ADC1          DMA1 通道 1    只给 ADC。F3 再真正开启
 * 其它通道禁用。不要给 SPI、I2C、PWM 分配通道号。
 */
#define DMA_ID_UART1_RX           5u
#define DMA_ID_ADC1               1u

#endif /* FW_APP_00_SYSTEM_SYSTEM_DMA_CONFIG_H_ */
