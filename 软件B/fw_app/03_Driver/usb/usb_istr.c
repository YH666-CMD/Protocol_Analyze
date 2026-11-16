/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_istr.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2022/12/28
 * Description        : ISTR events interrupt service routines
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/*
 * 相对原厂例程只改了两处，其余保持原样：
 *   1. 还没配置时收到挂起，只清掉 CNTR_RESUME，不调用 Resume。
 *      这个位会自己驱动总线，SOF 就进不来。
 *   2. 连续丢失 SOF 后的等待有上限。等不到复位就返回，不把中断卡死。
 * 打印由 usb_log.h 的宏决定。开关为 0 时，那两次调用不产生代码。
 */
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "usb_log.h"

uint16_t Ep0RxBlks;

/* Private variables */
__IO uint16_t wIstr;  
__IO uint8_t bIntPackSOF = 0;  
__IO uint32_t esof_counter =0;
__IO uint32_t wCNTR=0;

/* function pointers to non-control endpoints service routines */
void (*pEpInt_IN[7])(void) ={
	EP1_IN_Callback,
	EP2_IN_Callback,
	EP3_IN_Callback,
	EP4_IN_Callback,
	EP5_IN_Callback,
	EP6_IN_Callback,
	EP7_IN_Callback,
};

void (*pEpInt_OUT[7])(void) ={
	EP1_OUT_Callback,
	EP2_OUT_Callback,
	EP3_OUT_Callback,
	EP4_OUT_Callback,
	EP5_OUT_Callback,
	EP6_OUT_Callback,
	EP7_OUT_Callback,
};

/*******************************************************************************
 * @fn        USB_Istr
 *
 * @brief     ISTR events interrupt service routine
 *
 * @return    None.
 */
void USB_Istr(void)
{
  uint32_t i=0;
  __IO uint32_t EP[8];
  if ((*_pEPRxCount(0) & 0xFC00 )!= Ep0RxBlks)
  {
    *_pEPRxCount(0) |= (Ep0RxBlks & 0xFC00);
  }
  wIstr = _GetISTR();
#if (IMR_MSK & ISTR_SOF)
  if (wIstr & ISTR_SOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_SOF);
    bIntPackSOF++;

#ifdef SOF_CALLBACK
    SOF_Callback();
		
#endif
  }
#endif
 
  
#if (IMR_MSK & ISTR_CTR)
  if (wIstr & ISTR_CTR & wInterrupt_Mask)
  {
    CTR_LP();
#ifdef CTR_CALLBACK
    CTR_Callback();
#endif
  }
#endif
 
#if (IMR_MSK & ISTR_RESET)
  if (wIstr & ISTR_RESET & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_RESET);
    Device_Property.Reset();
    
#ifdef RESET_CALLBACK
    RESET_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_DOVR)
  if (wIstr & ISTR_DOVR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_DOVR);
#ifdef DOVR_CALLBACK
    DOVR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_ERR)
  if (wIstr & ISTR_ERR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ERR);
#ifdef ERR_CALLBACK
    ERR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_WKUP)
  if (wIstr & ISTR_WKUP & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_WKUP);
    Resume(RESUME_EXTERNAL);
#ifdef WKUP_CALLBACK
    WKUP_Callback();
#endif
  }
#endif
#if (IMR_MSK & ISTR_SUSP)
  if (wIstr & ISTR_SUSP & wInterrupt_Mask)
  {
    /* 还没配置完成时 Resume 会把 CNTR bit4 拉高，总线被自己驱动，主机的帧进不来。 */
    if ((bDeviceState == CONFIGURED) && fSuspendEnabled)
    {
      Suspend();
    }
    else
    {
      uint16_t wCntr;

      wCntr = _GetCNTR();
      wCntr &= (uint16_t)(~CNTR_RESUME);
      _SetCNTR(wCntr);
      usb_log_resume_blocked();
    }
    _SetISTR((uint16_t)CLR_SUSP);
#ifdef SUSP_CALLBACK
    SUSP_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_ESOF)
  if (wIstr & ISTR_ESOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ESOF);
    
    if ((_GetFNR()&FNR_RXDP)!=0)
    {
      esof_counter ++;
      
      if ((esof_counter >3)&&((_GetCNTR()&CNTR_FSUSP)==0))
      {           

        wCNTR = _GetCNTR(); 
      
        for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);
      
        wCNTR|=CNTR_FRES;
        _SetCNTR(wCNTR);
 
        wCNTR&=~CNTR_FRES;
        _SetCNTR(wCNTR);

        /* 复位可能不来。不能死等，也不能写 CLR_RESET，那会把别的标志一起清掉。 */
        {
            volatile uint32_t spin;

            for (spin = 0u; spin < 200000u; spin++)
            {
                if ((_GetISTR() & ISTR_RESET) != 0u)
                {
                    break;
                }
            }
            if ((_GetISTR() & ISTR_RESET) == 0u)
            {
                usb_log_esof_bail();
                esof_counter = 0;
            }
            else
            {
                _SetISTR((uint16_t)CLR_RESET);
                for (i = 0u; i < 8u; i++)
                {
                    _SetENDPOINT(i, EP[i]);
                }
                esof_counter = 0;
            }
        }
      }
    }
    else
    {
        esof_counter = 0;
    }
    
    Resume(RESUME_ESOF); 

#ifdef ESOF_CALLBACK
    ESOF_Callback();
#endif
  }
#endif
} /* USB_Istr */






