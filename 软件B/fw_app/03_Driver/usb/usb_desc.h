/*
 * usb_desc.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 描述符长度。内容在 usb_desc.c，和官方 SimulateCDC 一致。
 *
 * 厂商号和产品号来自 usb_cfg.h。默认 1A86:FE0C，类是通信设备。
 * 设备管理器里显示成串口。改身份只改 usb_cfg.h 里的 USB_VID、USB_PID。
 */
#ifndef FW_APP_03_DRIVER_USB_USB_DESC_H_
#define FW_APP_03_DRIVER_USB_USB_DESC_H_

#include "ch32v20x.h"
#include "usb_cfg.h"

#define USB_DEVICE_DESCRIPTOR_TYPE          0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE   0x02
#define USB_STRING_DESCRIPTOR_TYPE          0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE       0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE        0x05

#define DEF_USBD_UEP0_SIZE                  USB_EP0_SIZE
#define DEF_USBD_MAX_PACK_SIZE              USB_EP_MPS

#define USBD_SIZE_DEVICE_DESC               18
#define USBD_SIZE_CONFIG_DESC               67
#define USBD_SIZE_STRING_LANGID             4
#define USBD_SIZE_STRING_VENDOR             14
#define USBD_SIZE_STRING_PRODUCT            22
#define USBD_SIZE_STRING_SERIAL             22

extern const uint8_t USBD_DeviceDescriptor[USBD_SIZE_DEVICE_DESC];
extern const uint8_t USBD_ConfigDescriptor[USBD_SIZE_CONFIG_DESC];
extern const uint8_t USBD_StringLangID[USBD_SIZE_STRING_LANGID];
extern const uint8_t USBD_StringVendor[USBD_SIZE_STRING_VENDOR];
extern const uint8_t USBD_StringProduct[USBD_SIZE_STRING_PRODUCT];
extern uint8_t USBD_StringSerial[USBD_SIZE_STRING_SERIAL];

#endif /* FW_APP_03_DRIVER_USB_USB_DESC_H_ */
