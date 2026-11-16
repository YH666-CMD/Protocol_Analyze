/*
 * usb_desc.c
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 设备描述符。字节和官方 SimulateCDC 相同，主机靠它枚举。
 */
#include "usb_desc.h"
#include "usb_cfg.h"

const uint8_t USBD_DeviceDescriptor[] =
{
    USBD_SIZE_DEVICE_DESC,
    0x01,
    0x10, 0x01,
    0x02,
    0x00,
    0x00,
    DEF_USBD_UEP0_SIZE,
    (uint8_t)(USB_VID & 0xFFu), (uint8_t)(USB_VID >> 8),
    (uint8_t)(USB_PID & 0xFFu), (uint8_t)(USB_PID >> 8),
    0x00, 0x01,
    0x01,
    0x02,
    0x00,
    0x01,
};

const uint8_t USBD_ConfigDescriptor[] =
{
    0x09,
    0x02,
    USBD_SIZE_CONFIG_DESC & 0xFF, USBD_SIZE_CONFIG_DESC >> 8,
    0x02,
    0x01,
    0x00,
    0x80,
    0x32,

    0x09,
    0x04,
    0x00,
    0x00,
    0x01,
    0x02,
    0x02,
    0x01,
    0x00,

    0x05, 0x24, 0x00, 0x10, 0x01,
    0x05, 0x24, 0x01, 0x00, 0x01,
    0x04, 0x24, 0x02, 0x02,
    0x05, 0x24, 0x06, 0x00, 0x01,

    0x07,
    0x05,
    0x81,
    0x03,
    (uint8_t)(USB_EP_MPS & 0xFFu), (uint8_t)(USB_EP_MPS >> 8),
    0x01,

    0x09,
    0x04,
    0x01,
    0x00,
    0x02,
    0x0A,
    0x00,
    0x00,
    0x00,

    0x07,
    0x05,
    0x02,
    0x02,
    (uint8_t)(USB_EP_MPS & 0xFFu), (uint8_t)(USB_EP_MPS >> 8),
    0x00,

    0x07,
    0x05,
    0x83,
    0x02,
    (uint8_t)(USB_EP_MPS & 0xFFu), (uint8_t)(USB_EP_MPS >> 8),
    0x00,
};

const uint8_t USBD_StringLangID[USBD_SIZE_STRING_LANGID] =
{
    USBD_SIZE_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
};

const uint8_t USBD_StringVendor[USBD_SIZE_STRING_VENDOR] =
{
    USBD_SIZE_STRING_VENDOR,
    USB_STRING_DESCRIPTOR_TYPE,
    'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0
};

const uint8_t USBD_StringProduct[USBD_SIZE_STRING_PRODUCT] =
{
    USBD_SIZE_STRING_PRODUCT,
    USB_STRING_DESCRIPTOR_TYPE,
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0
};

uint8_t USBD_StringSerial[USBD_SIZE_STRING_SERIAL] =
{
    USBD_SIZE_STRING_SERIAL,
    USB_STRING_DESCRIPTOR_TYPE,
    '0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0
};
