/*
 * usb_prop.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 端点 0 的回调。名字是原厂库规定的，不能改。
 */
#ifndef FW_APP_03_DRIVER_USB_USB_PROP_H_
#define FW_APP_03_DRIVER_USB_USB_PROP_H_

#include "usb_core.h"

#define CDC_GET_LINE_CODING         0x21
#define CDC_SET_LINE_CODING         0x20
#define CDC_SET_LINE_CTLSTE         0x22
#define CDC_SEND_BREAK              0x23

#define USBD_GetConfiguration       NOP_Process
#define USBD_GetInterface           NOP_Process
#define USBD_SetInterface           NOP_Process
#define USBD_GetStatus              NOP_Process
#define USBD_SetEndPointFeature     NOP_Process

/*
 * 下面这些名字由原厂库查表调用，不能改，也不能改成 static。
 * USBD_init：断开上拉再接上，让主机重新枚举。配置完成前禁止挂起。
 * USBD_Reset：总线复位后把端点 0、1、2、3 配回初始状态，地址回到 0。
 * USBD_SetDeviceAddress：主机分配了地址。
 * USBD_SetConfiguration：主机选定非 0 配置后才允许挂起。
 * USBD_Status_In：控制传输的状态阶段。SET_LINE_CODING 的 7 字节在这之前已经进内部缓冲。
 * USBD_Data_Setup / USBD_NoData_Setup：只收下 CDC 线路请求，标准请求仍走原厂。
 */
void USBD_init(void);
void USBD_Reset(void);
void USBD_SetConfiguration(void);
void USBD_SetDeviceAddress(void);
void USBD_SetDeviceFeature(void);
void USBD_ClearFeature(void);
void USBD_Status_In(void);
void USBD_Status_Out(void);
RESULT USBD_Data_Setup(uint8_t RequestNo);
RESULT USBD_NoData_Setup(uint8_t RequestNo);
RESULT USBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *USBD_GetDeviceDescriptor(uint16_t Length);
uint8_t *USBD_GetConfigDescriptor(uint16_t Length);
uint8_t *USBD_GetStringDescriptor(uint16_t Length);
uint8_t USBD_ENDPx_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len);

/* 端点 3 现在还能接多少字节。0 表示上一包还在发送。 */
uint16_t usb_ep3_room(void);

/* 主机通过类请求写下的 7 字节线路参数。还没下发时是 115200 8N1。 */
void usb_cdc_line_get(uint8_t out[7]);

/* 登记端点 2 收到数据后的回调。fn 为 NULL 就丢掉。回调在 USB 中断里，不能等待。 */
void usb_rx_hook_set(void (*fn)(const uint8_t *data, uint16_t len));
void (*usb_rx_hook_get(void))(const uint8_t *data, uint16_t len);

#endif /* FW_APP_03_DRIVER_USB_USB_PROP_H_ */
