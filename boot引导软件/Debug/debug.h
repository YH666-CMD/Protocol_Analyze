/********************************** (C) COPYRIGHT  *******************************
 * 文件: debug.h
 * 说明: 延时与串口 printf 调试接口
 *******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#include "stdio.h"
#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 选择 printf 输出使用的串口 */
#define DEBUG_UART1    1
#define DEBUG_UART2    2
#define DEBUG_UART3    3

#ifndef DEBUG
#define DEBUG   DEBUG_UART1    /* 默认 USART1（PA9 发送） */
#endif

/* SDI 仿真打印开关（一般关闭，用串口即可） */
#define SDI_PR_CLOSE   0
#define SDI_PR_OPEN    1

#ifndef SDI_PRINT
#define SDI_PRINT   SDI_PR_CLOSE
#endif

void Delay_Init(void);
void Delay_Us(uint32_t n);
void Delay_Ms(uint32_t n);
void USART_Printf_Init(uint32_t baudrate);
void SDI_Printf_Enable(void);

#if(DEBUG)
  #define PRINT(format, ...)    printf(format, ##__VA_ARGS__)
#else
  #define PRINT(X...)
#endif

#ifdef __cplusplus
}
#endif

#endif
