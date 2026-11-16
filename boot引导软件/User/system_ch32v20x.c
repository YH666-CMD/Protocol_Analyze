/********************************** (C) COPYRIGHT *******************************
 * 文件               : system_ch32v20x.c
 * 作者               : WCH
 * 版本               : V1.0.0
 * 日期               : 2021/06/06
 * 说明               : CH32V203C8T6（D6）系统时钟。
 *                      Boot 不升主频。SystemInit 只把 RCC 恢复成复位默认，
 *                      时钟停在 HSI。App 自己再升频。
*********************************************************************************
* 版权所有 (c) 2021 南京沁恒微电子股份有限公司
*******************************************************************************/
#include "ch32v20x.h"

uint32_t SystemCoreClock = HSI_VALUE;

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

/*********************************************************************
 * SystemInit — 复位后把 RCC 恢复成默认状态，不升频
 *********************************************************************/
void SystemInit(void)
{
    RCC->CTLR |= (uint32_t)0x00000001;
    RCC->CFGR0 &= (uint32_t)0xF0FF0000;
    RCC->CTLR &= (uint32_t)0xFEF6FFFF;
    RCC->CTLR &= (uint32_t)0xFFFBFFFF;
    RCC->CFGR0 &= (uint32_t)0xFF00FFFF;
    RCC->INTR = 0x009F0000;
}

/*********************************************************************
 * SystemCoreClockUpdate — 根据 RCC 寄存器刷新 SystemCoreClock
 *********************************************************************/
void SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0, pllmull = 0, pllsource = 0;

    tmp = RCC->CFGR0 & RCC_SWS;

    switch (tmp)
    {
        case 0x00:
            SystemCoreClock = HSI_VALUE;
            break;
        case 0x04:
            SystemCoreClock = HSE_VALUE;
            break;
        case 0x08:
            pllmull = RCC->CFGR0 & RCC_PLLMULL;
            pllsource = RCC->CFGR0 & RCC_PLLSRC;
            pllmull = (pllmull >> 18) + 2;

            if (pllmull == 17)
            {
                pllmull = 18;
            }

            if (pllsource == 0x00)
            {
                if (EXTEN->EXTEN_CTR & EXTEN_PLL_HSI_PRE)
                {
                    SystemCoreClock = HSI_VALUE * pllmull;
                }
                else
                {
                    SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
                }
            }
            else if ((RCC->CFGR0 & RCC_PLLXTPRE) != (uint32_t)RESET)
            {
                SystemCoreClock = (HSE_VALUE >> 1) * pllmull;
            }
            else
            {
                SystemCoreClock = HSE_VALUE * pllmull;
            }
            break;
        default:
            SystemCoreClock = HSI_VALUE;
            break;
    }

    tmp = AHBPrescTable[((RCC->CFGR0 & RCC_HPRE) >> 4)];
    SystemCoreClock >>= tmp;
}
