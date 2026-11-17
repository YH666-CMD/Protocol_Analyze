/********************************** (C) COPYRIGHT *******************************
 * 文件: ch32v20x_it.c
 * 说明: 中断服务函数（异常处理）
 *******************************************************************************/
#include "ch32v20x_it.h"
#include "log.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/* 不可屏蔽中断：严重硬件异常时进入，停在此处便于调试 */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/* 硬错误：先把出错地址记进共享区，再复位。软复位后这条还在。 */
void HardFault_Handler(void)
{
  uint32_t mepc;

  __asm volatile ("csrr %0, mepc" : "=r"(mepc));
  (void)log_sw_driver.pf_emergency((uint32_t)PLAT_ERR_HARDFAULT, mepc);
  NVIC_SystemReset();
  while (1)
  {
  }
}
