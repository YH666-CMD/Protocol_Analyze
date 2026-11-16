/********************************** (C) COPYRIGHT *******************************
 * 文件: ch32v20x_it.c
 * 说明: 中断服务函数（异常处理）
 *******************************************************************************/
#include "ch32v20x_it.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/* 不可屏蔽中断：严重硬件异常时进入，停在此处便于调试 */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/* 硬错误：复位 MCU，避免程序跑飞 */
void HardFault_Handler(void)
{
  NVIC_SystemReset();
  while (1)
  {
  }
}
