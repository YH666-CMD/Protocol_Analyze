/********************************** (C) COPYRIGHT *******************************
 * 文件: system_ch32v20x.h
 * 说明: 系统时钟初始化接口
 *******************************************************************************/
#ifndef __SYSTEM_ch32v20x_H 
#define __SYSTEM_ch32v20x_H

#ifdef __cplusplus
 extern "C" {
#endif 

extern uint32_t SystemCoreClock;    /* 当前系统内核时钟频率（Hz） */

extern void SystemInit(void);           /* 启动时配置时钟 */
extern void SystemCoreClockUpdate(void); /* 从 RCC 寄存器刷新 SystemCoreClock */

#ifdef __cplusplus
}
#endif

#endif
