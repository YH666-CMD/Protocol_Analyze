/*
 * hw_config.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 原厂 USB 库要调用的板级函数。声明必须和库里的名字一致。
 */
#ifndef FW_APP_03_DRIVER_USB_HW_CONFIG_H_
#define FW_APP_03_DRIVER_USB_HW_CONFIG_H_

#include "usb_type.h"

/*
 * 这三个名字是原厂电源管理和初始化要调用的，不能改。
 * 实现在 drv_usb.c。
 */

/* 主机要求挂起。只改设备状态，不关系统时钟，也不进 WFI。 */
void Enter_LowPowerMode(void);

/* 从挂起回来。已经有配置就回到已配置，否则回到已连接。 */
void Leave_LowPowerMode(void);

/*
 * 控制 D+ 上拉。
 * NewState 为 ENABLE 时把 PA11、PA12 交给 USB，并退出掉电。
 * Pin_In_IPU 为 ENABLE 时打开片内 1.5k 上拉。是否打开由 USB_USE_INTERNAL_PU 决定。
 */
void USB_Port_Set(FunctionalState NewState, FunctionalState Pin_In_IPU);

#endif /* FW_APP_03_DRIVER_USB_HW_CONFIG_H_ */
