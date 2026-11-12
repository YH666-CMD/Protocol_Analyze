/********************************** (C) COPYRIGHT *******************************
 * 文件               : system_ch32v20x.c
 * 作者               : WCH
 * 版本               : V1.0.0
 * 日期               : 2021/06/06
 * 说明               : CH32V203C8T6（D6）系统时钟。
 *                      HSE = 8MHz，PLL×18 = 144MHz；HSE 失败时回退 HSI×18。
*********************************************************************************
* 版权所有 (c) 2021 南京沁恒微电子股份有限公司
*******************************************************************************/
#include "ch32v20x.h"

uint32_t SystemCoreClock = 144000000;

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

static void SetSysClock(void);
static void SetSysClockTo144_HSI_Fallback(void);
static void SetSysClockTo144_HSE(void);

/*********************************************************************
 * SystemInit — 复位后初始化 RCC，并把系统时钟升到 144MHz
 *********************************************************************/
void SystemInit(void)
{
    RCC->CTLR |= (uint32_t)0x00000001;
    RCC->CFGR0 &= (uint32_t)0xF0FF0000;
    RCC->CTLR &= (uint32_t)0xFEF6FFFF;
    RCC->CTLR &= (uint32_t)0xFFFBFFFF;
    RCC->CFGR0 &= (uint32_t)0xFF00FFFF;
    RCC->INTR = 0x009F0000;
    SetSysClock();
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

static void SetSysClock(void)
{
    SetSysClockTo144_HSE();
}

/* HSE 起振失败时，用 HSI×18 升到 144MHz */
static void SetSysClockTo144_HSI_Fallback(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL18);

    RCC->CTLR |= RCC_PLLON;
    while ((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }

    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}

/* 优先 HSE×18 = 144MHz，失败则回退内部 HSI */
static void SetSysClockTo144_HSE(void)
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    RCC->CTLR |= ((uint32_t)RCC_HSEON);

    do
    {
        HSEStatus = RCC->CTLR & RCC_HSERDY;
        StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CTLR & RCC_HSERDY) != RESET)
    {
        HSEStatus = (uint32_t)0x01;
    }
    else
    {
        HSEStatus = (uint32_t)0x00;
    }

    if (HSEStatus == (uint32_t)0x01)
    {
        RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
        RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
        RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

        RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));
        RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL18);

        RCC->CTLR |= RCC_PLLON;
        while ((RCC->CTLR & RCC_PLLRDY) == 0)
        {
        }

        RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
        RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
        while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
        {
        }
    }
    else
    {
        SetSysClockTo144_HSI_Fallback();
    }
}
