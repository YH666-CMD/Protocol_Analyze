/********************************** (C) COPYRIGHT *******************************
 * 文件               : system_ch32v20x.c
 * 作者               : WCH
 * 版本               : V1.0.0
 * 日期               : 2021/06/06
 * 说明               : 系统时钟固定为 HSE 8MHz × 12 = 96MHz。
 *                      不再按寄存器反推频率，HSE 没起来也不再自动升到 96MHz。
*********************************************************************************
* 版权所有 (c) 2021 南京沁恒微电子股份有限公司
*******************************************************************************/
#include "ch32v20x.h"

#define SYSCLK_HZ              96000000u

uint32_t SystemCoreClock = SYSCLK_HZ;

/* 只走这一条：外部 8MHz，锁相环 12 倍。APB1 二分频是 48MHz。 */
static void set_sysclk_96(void)
{
    __IO uint32_t n;
    uint32_t ready;

    RCC->CTLR |= (uint32_t)RCC_HSEON;
    n = 0u;
    do
    {
        ready = RCC->CTLR & RCC_HSERDY;
        n++;
    } while ((ready == 0u) && (n != HSE_STARTUP_TIMEOUT));

    if ((RCC->CTLR & RCC_HSERDY) == 0u)
    {
        return;
    }

    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;
    RCC->CFGR0 &= (uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL);
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL12);
    RCC->CTLR |= RCC_PLLON;
    while ((RCC->CTLR & RCC_PLLRDY) == 0u)
    {
    }
    RCC->CFGR0 &= (uint32_t)~(RCC_SW);
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08u)
    {
    }
}

void SystemInit(void)
{
    RCC->CTLR |= (uint32_t)0x00000001;
    RCC->CFGR0 &= (uint32_t)0xF0FF0000;
    RCC->CTLR &= (uint32_t)0xFEF6FFFF;
    RCC->CTLR &= (uint32_t)0xFFFBFFFF;
    RCC->CFGR0 &= (uint32_t)0xFF00FFFF;
    RCC->INTR = 0x009F0000;
    set_sysclk_96();
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = SYSCLK_HZ;
}
