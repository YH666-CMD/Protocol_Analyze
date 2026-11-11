/********************************** (C) COPYRIGHT *******************************
 * 文件               : system_ch32v20x.c
 * 作者               : WCH
 * 版本               : V1.0.0
 * 日期               : 2021/06/06
 * 说明               : 系统时钟配置（启动时由 SystemInit 调用）
 *                      D6 系列（C8T6 等）外部晶振 HSE = 8MHz
 *                      D8/D8W 系列外部晶振 HSE = 32MHz
*********************************************************************************
* 版权所有 (c) 2021 南京沁恒微电子股份有限公司
*******************************************************************************/
#include "ch32v20x.h" 

/*
 * 系统时钟选择：只保留一行 #define，其余注释掉。
 * 复位后默认 HSI 8MHz；若下面全部注释，则一直使用 HSI。
 */
//#define SYSCLK_FREQ_HSE    HSE_VALUE
//#define SYSCLK_FREQ_48MHz_HSE  48000000
//#define SYSCLK_FREQ_56MHz_HSE  56000000
//#define SYSCLK_FREQ_72MHz_HSE  72000000
//#define SYSCLK_FREQ_96MHz_HSE  96000000  /* C8T6(D6): HSE 8MHz × PLL12 = 96MHz */
//#define SYSCLK_FREQ_120MHz_HSE  120000000
#define SYSCLK_FREQ_144MHz_HSE  144000000  /* C8T6(D6): HSE 8MHz × PLL18 = 144MHz (最高) */
//#define SYSCLK_FREQ_HSI    HSI_VALUE
//#define SYSCLK_FREQ_48MHz_HSI  48000000
//#define SYSCLK_FREQ_56MHz_HSI  56000000
//#define SYSCLK_FREQ_72MHz_HSI  72000000
//#define SYSCLK_FREQ_96MHz_HSI  96000000
//#define SYSCLK_FREQ_120MHz_HSI  120000000
//#define SYSCLK_FREQ_144MHz_HSI  144000000

/* 根据所选宏，设定 SystemCoreClock 初始值 */
#ifdef SYSCLK_FREQ_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_HSE;              /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_48MHz_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_48MHz_HSE;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_56MHz_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_56MHz_HSE;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_72MHz_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_72MHz_HSE;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_96MHz_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_96MHz_HSE;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_120MHz_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_120MHz_HSE;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_144MHz_HSE
uint32_t SystemCoreClock         = SYSCLK_FREQ_144MHz_HSE;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_48MHz_HSI
uint32_t SystemCoreClock         = SYSCLK_FREQ_48MHz_HSI;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_56MHz_HSI
uint32_t SystemCoreClock         = SYSCLK_FREQ_56MHz_HSI;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_72MHz_HSI
uint32_t SystemCoreClock         = SYSCLK_FREQ_72MHz_HSI;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_96MHz_HSI
uint32_t SystemCoreClock         = SYSCLK_FREQ_96MHz_HSI;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_120MHz_HSI
uint32_t SystemCoreClock         = SYSCLK_FREQ_120MHz_HSI;        /* 系统内核时钟频率 */
#elif defined SYSCLK_FREQ_144MHz_HSI
uint32_t SystemCoreClock         = SYSCLK_FREQ_144MHz_HSI;        /* 系统内核时钟频率 */
#else
uint32_t SystemCoreClock         = HSI_VALUE;                    /* 系统内核时钟频率 */

#endif

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};


/* 内部时钟配置函数声明 */
static void SetSysClock(void);

#ifdef SYSCLK_FREQ_HSE
static void SetSysClockToHSE( void );
#elif defined SYSCLK_FREQ_48MHz_HSE
static void SetSysClockTo48_HSE( void );
#elif defined SYSCLK_FREQ_56MHz_HSE
static void SetSysClockTo56_HSE( void );
#elif defined SYSCLK_FREQ_72MHz_HSE
static void SetSysClockTo72_HSE( void );
#elif defined SYSCLK_FREQ_96MHz_HSE
static void SetSysClockTo96_HSE( void );
#elif defined SYSCLK_FREQ_120MHz_HSE
static void SetSysClockTo120_HSE( void );
#elif defined SYSCLK_FREQ_144MHz_HSE
static void SetSysClockTo144_HSE( void );
#elif defined SYSCLK_FREQ_48MHz_HSI
static void SetSysClockTo48_HSI( void );
#elif defined SYSCLK_FREQ_56MHz_HSI
static void SetSysClockTo56_HSI( void );
#elif defined SYSCLK_FREQ_72MHz_HSI
static void SetSysClockTo72_HSI( void );
#elif defined SYSCLK_FREQ_96MHz_HSI
static void SetSysClockTo96_HSI( void );
#elif defined SYSCLK_FREQ_120MHz_HSI
static void SetSysClockTo120_HSI( void );
#elif defined SYSCLK_FREQ_144MHz_HSI
static void SetSysClockTo144_HSI( void );

#endif

/*********************************************************************
 * SystemInit — 复位后初始化 RCC，配置 PLL 与系统时钟
 *********************************************************************/
void SystemInit (void)
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
 * SystemCoreClockUpdate — 根据 RCC 寄存器刷新 SystemCoreClock 变量
 *********************************************************************/
void SystemCoreClockUpdate (void)
{
  uint32_t tmp = 0, pllmull = 0, pllsource = 0, Pll_6_5 = 0;

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
      pllmull = ( pllmull >> 18) + 2;

      if(pllmull == 17) pllmull = 18;

      if (pllsource == 0x00)
      {
          if(EXTEN->EXTEN_CTR & EXTEN_PLL_HSI_PRE){
              SystemCoreClock = HSI_VALUE * pllmull;
          }
          else{
              SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
          }
      }
      else
      {
#if defined (CH32V20x_D8W) || defined (CH32V20x_D8)
        if(((RCC->CFGR0 & (3<<22)) == (3<<22)) && (RCC_USB5PRE_JUDGE()== SET))
        {
          SystemCoreClock = ((HSE_VALUE>>1)) * pllmull;
        }
        else
#endif
        if ((RCC->CFGR0 & RCC_PLLXTPRE) != (uint32_t)RESET)
        {
#if defined (CH32V20x_D8) || defined (CH32V20x_D8W)
          SystemCoreClock = ((HSE_VALUE>>2) >> 1) * pllmull;
#else
          SystemCoreClock = (HSE_VALUE >> 1) * pllmull;
#endif
        }
        else
        {
#if defined (CH32V20x_D8) || defined (CH32V20x_D8W)
            SystemCoreClock = (HSE_VALUE>>2) * pllmull;
#else
          SystemCoreClock = HSE_VALUE * pllmull;
#endif
        }
      }

      if(Pll_6_5 == 1) SystemCoreClock = (SystemCoreClock / 2);

      break;
    default:
      SystemCoreClock = HSI_VALUE;
      break;
  }

  tmp = AHBPrescTable[((RCC->CFGR0 & RCC_HPRE) >> 4)];
  SystemCoreClock >>= tmp;
}

/*********************************************************************
 * SetSysClock — 按宏定义调用对应的时钟配置函数
 *********************************************************************/
static void SetSysClock(void)
{
  /* 未使用 GPIO 引脚默认下拉（可选） */
  //GPIO_IPD_Unused();
#ifdef SYSCLK_FREQ_HSE
    SetSysClockToHSE();
#elif defined SYSCLK_FREQ_48MHz_HSE
    SetSysClockTo48_HSE();
#elif defined SYSCLK_FREQ_56MHz_HSE
    SetSysClockTo56_HSE();
#elif defined SYSCLK_FREQ_72MHz_HSE
    SetSysClockTo72_HSE();
#elif defined SYSCLK_FREQ_96MHz_HSE
    SetSysClockTo96_HSE();
#elif defined SYSCLK_FREQ_120MHz_HSE
    SetSysClockTo120_HSE();
#elif defined SYSCLK_FREQ_144MHz_HSE
    SetSysClockTo144_HSE();
#elif defined SYSCLK_FREQ_48MHz_HSI
    SetSysClockTo48_HSI();
#elif defined SYSCLK_FREQ_56MHz_HSI
    SetSysClockTo56_HSI();
#elif defined SYSCLK_FREQ_72MHz_HSI
    SetSysClockTo72_HSI();
#elif defined SYSCLK_FREQ_96MHz_HSI
    SetSysClockTo96_HSI();
#elif defined SYSCLK_FREQ_120MHz_HSI
    SetSysClockTo120_HSI();
#elif defined SYSCLK_FREQ_144MHz_HSI
    SetSysClockTo144_HSI();

#endif
 
 /* 若上面所有宏都未启用，则保持复位默认的 HSI 8MHz */
}


#ifdef SYSCLK_FREQ_HSE

/* SetSysClockToHSE — 直接使用外部晶振 HSE 作为系统时钟 */
static void SetSysClockToHSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
  
   
  RCC->CTLR |= ((uint32_t)RCC_HSEON);
 
  /* 等待 HSE 就绪，超时则退出 */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

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
    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;      
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV1;
    
    /* 选择 HSE 作为系统时钟（D6: 8MHz，D8/D8W: 32MHz） */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_HSE;    

    /* 等待 HSE 成为系统时钟 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x04)
    {
    }
  }
  else
  { 
		/* HSE 起振失败，时钟配置无效 */
  }  
}

#elif defined SYSCLK_FREQ_48MHz_HSE

/* SetSysClockTo48_HSE — HSE + PLL，系统时钟 48MHz */
static void SetSysClockTo48_HSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
     
   
  RCC->CTLR |= ((uint32_t)RCC_HSEON);
  /* 等待 HSE 就绪，超时则退出 */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

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
    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;    
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;  
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* D6: PLL = HSE×6 = 48MHz；D8/D8W: HSE/4×6 */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

     RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL6);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;    
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
  }
  else
  { 
		/* HSE 起振失败，时钟配置无效 */
  } 
}

#elif defined SYSCLK_FREQ_56MHz_HSE

/* SetSysClockTo56_HSE — HSE + PLL，系统时钟 56MHz */
static void SetSysClockTo56_HSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
     
  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* 等待 HSE 就绪，超时则退出 */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

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
    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;   
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;
  
    /* D6: PLL = HSE×7 = 56MHz；D8/D8W: HSE/4×7 */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL7);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }

    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;    
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
  }
  else
  { 
		/* HSE 起振失败，时钟配置无效 */
  } 
}

#elif defined SYSCLK_FREQ_72MHz_HSE

/* SetSysClockTo72_HSE — HSE + PLL，系统时钟 72MHz */
static void SetSysClockTo72_HSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
     
  RCC->CTLR |= ((uint32_t)RCC_HSEON);
 
  /* 等待 HSE 就绪，超时则退出 */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

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
    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1; 
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1; 
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;
 
    /* D6: PLL = HSE×9 = 72MHz；D8/D8W: HSE/4×9 */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE |
                                        RCC_PLLMULL));

    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL9);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }    
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;    
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
  }
  else
  { 
		/* HSE 起振失败，时钟配置无效 */
  }
}


#elif defined SYSCLK_FREQ_96MHz_HSE

/* SetSysClockTo96_HSE — HSE + PLL，系统时钟 96MHz */
static void SetSysClockTo96_HSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* 等待 HSE 就绪，超时则退出 */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

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
    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* D6: PLL = HSE×12 = 96MHz；D8/D8W: HSE/4×12 */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE |
                                        RCC_PLLMULL));

    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL12);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
  }
  else
  {
        /* HSE 起振失败，时钟配置无效 */
  }
}


#elif defined SYSCLK_FREQ_120MHz_HSE

/* SetSysClockTo120_HSE — HSE + PLL，系统时钟 120MHz */
static void SetSysClockTo120_HSE(void)
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    RCC->CTLR |= ((uint32_t)RCC_HSEON);

    /* 等待 HSE 就绪，超时则退出 */
    do
    {
        HSEStatus = RCC->CTLR & RCC_HSERDY;
        StartUpCounter++;
    } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if((RCC->CTLR & RCC_HSERDY) != RESET)
    {
        HSEStatus = (uint32_t)0x01;
    }
    else
    {
        HSEStatus = (uint32_t)0x00;
    }

    if(HSEStatus == (uint32_t)0x01)
    {
#if defined (CH32V20x_D8W)
        RCC->CFGR0 |= (uint32_t)(3<<22);
        /* HCLK = SYSCLK / 2（D8W 专用） */
        RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV2;
#else
        /* 内核时钟 HCLK = 系统时钟 */
        RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
#endif
        /* APB2 时钟 PCLK2 = HCLK */
        RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
        /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
        RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

        /* D6: PLL = HSE×15 = 120MHz；D8W: HSE/2×15 = 240MHz 等 */
        RCC->CFGR0 &= (uint32_t)((uint32_t) ~(RCC_PLLSRC | RCC_PLLXTPRE |
                                              RCC_PLLMULL));

        RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL15);

        /* 使能 PLL */
        RCC->CTLR |= RCC_PLLON;
        /* 等待 PLL 锁定 */
        while((RCC->CTLR & RCC_PLLRDY) == 0)
        {
        }
        /* 切换系统时钟到 PLL */
        RCC->CFGR0 &= (uint32_t)((uint32_t) ~(RCC_SW));
        RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
        /* 等待时钟源切换完成 */
        while((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
        {
        }
    }
    else
    {
        /* HSE 起振失败，时钟配置无效 */
    }
}
#elif defined SYSCLK_FREQ_144MHz_HSE

/* SetSysClockTo144_HSI_Fallback — HSE 失败时，HSI + PLL 升至 144MHz */
static void SetSysClockTo144_HSI_Fallback(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL18);

    RCC->CTLR |= RCC_PLLON;
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }

    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    while((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}

/* SetSysClockTo144_HSE — 优先 HSE×18=144MHz，失败则调用 HSI 回退 */
static void SetSysClockTo144_HSE(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  RCC->CTLR |= ((uint32_t)RCC_HSEON);

  /* 等待 HSE 就绪，超时则退出 */
  do
  {
    HSEStatus = RCC->CTLR & RCC_HSERDY;
    StartUpCounter++;
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

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
    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* D6: PLL = HSE×18 = 144MHz（HSE=8MHz）；D8/D8W: HSE/4×18 */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE |
                                        RCC_PLLMULL));

    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSE | RCC_PLLXTPRE_HSE | RCC_PLLMULL18);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
  }
  else
  {
        /* 外部 8MHz 晶振未起振时，改用 HSI 倍频至 144MHz */
        SetSysClockTo144_HSI_Fallback();
  }
}

#elif defined SYSCLK_FREQ_48MHz_HSI

/* SetSysClockTo48_HSI — 内部 HSI + PLL，系统时钟 48MHz */
static void SetSysClockTo48_HSI(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* PLL 配置：HSI × 6 = 48MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

     RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL6);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}

#elif defined SYSCLK_FREQ_56MHz_HSI

/* SetSysClockTo56_HSI — 内部 HSI + PLL，系统时钟 56MHz */
static void SetSysClockTo56_HSI(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* PLL 配置：HSI × 7 = 56MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

     RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL7);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}

#elif defined SYSCLK_FREQ_72MHz_HSI

/* SetSysClockTo72_HSI — 内部 HSI + PLL，系统时钟 72MHz */
static void SetSysClockTo72_HSI(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* PLL 配置：HSI × 9 = 72MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

     RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL9);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}


#elif defined SYSCLK_FREQ_96MHz_HSI

/* SetSysClockTo96_HSI — 内部 HSI + PLL，系统时钟 96MHz */
static void SetSysClockTo96_HSI(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* PLL 配置：HSI × 12 = 96MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

     RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL12);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}


#elif defined SYSCLK_FREQ_120MHz_HSI

/* SetSysClockTo120_HSI — 内部 HSI + PLL，系统时钟 120MHz */
static void SetSysClockTo120_HSI(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* PLL 配置：HSI × 15 = 120MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t) ~(RCC_PLLSRC | RCC_PLLXTPRE |
                                          RCC_PLLMULL));

    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL15);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t) ~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}
#elif defined SYSCLK_FREQ_144MHz_HSI

/* SetSysClockTo144_HSI — 内部 HSI + PLL，系统时钟 144MHz */
static void SetSysClockTo144_HSI(void)
{
    EXTEN->EXTEN_CTR |= EXTEN_PLL_HSI_PRE;

    /* 内核时钟 HCLK = 系统时钟 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;
    /* APB2 时钟 PCLK2 = HCLK */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE2_DIV1;
    /* APB1 时钟（通常二分频，见 RCC_PPRE1 宏） */
    RCC->CFGR0 |= (uint32_t)RCC_PPRE1_DIV2;

    /* PLL 配置：HSI × 18 = 144MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC | RCC_PLLXTPRE | RCC_PLLMULL));

     RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Div2 | RCC_PLLMULL18);

    /* 使能 PLL */
    RCC->CTLR |= RCC_PLLON;
    /* 等待 PLL 锁定 */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* 切换系统时钟到 PLL */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* 等待时钟源切换完成 */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
}


#endif
