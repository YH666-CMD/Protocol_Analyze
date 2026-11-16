/*
 * usb_prop.c
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 端点 0。枚举、描述符、CDC 线路请求。
 *
 * 线路请求只写进 s_line，不改 USART1。和串口怎么对应，以后由上层做。
 * 回调名字是原厂库规定的，不能改。
 */
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_log.h"
#include "usb_cfg.h"
#include "hw_config.h"
#include "drv_usb.h"

extern uint8_t USBD_Endp3_Busy;

/* 默认 115200 8N1。波特率小端。主机没下发之前读到的就是这组。 */
static uint8_t s_line[7] =
{
    0x00, 0xC2, 0x01, 0x00,
    0x00,
    0x00,
    0x08
};

static usb_rx_fn s_rx_hook;

DEVICE Device_Table =
{
    EP_NUM,
    1
};

DEVICE_PROP Device_Property =
{
    USBD_init,
    USBD_Reset,
    USBD_Status_In,
    USBD_Status_Out,
    USBD_Data_Setup,
    USBD_NoData_Setup,
    USBD_Get_Interface_Setting,
    USBD_GetDeviceDescriptor,
    USBD_GetConfigDescriptor,
    USBD_GetStringDescriptor,
    0,
    DEF_USBD_UEP0_SIZE
};

USER_STANDARD_REQUESTS User_Standard_Requests =
{
    USBD_GetConfiguration,
    USBD_SetConfiguration,
    USBD_GetInterface,
    USBD_SetInterface,
    USBD_GetStatus,
    USBD_ClearFeature,
    USBD_SetEndPointFeature,
    USBD_SetDeviceFeature,
    USBD_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor =
{
    (uint8_t *)USBD_DeviceDescriptor,
    USBD_SIZE_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor =
{
    (uint8_t *)USBD_ConfigDescriptor,
    USBD_SIZE_CONFIG_DESC
};

ONE_DESCRIPTOR String_Descriptor[4] =
{
    {(uint8_t *)USBD_StringLangID, USBD_SIZE_STRING_LANGID},
    {(uint8_t *)USBD_StringVendor, USBD_SIZE_STRING_VENDOR},
    {(uint8_t *)USBD_StringProduct, USBD_SIZE_STRING_PRODUCT},
    {(uint8_t *)USBD_StringSerial, USBD_SIZE_STRING_SERIAL}
};

void usb_cdc_line_get(uint8_t out[7])
{
    uint8_t i;

    if (out == NULL)
    {
        return;
    }
    for (i = 0u; i < 7u; i++)
    {
        out[i] = s_line[i];
    }
}

void usb_rx_hook_set(usb_rx_fn fn)
{
    s_rx_hook = fn;
}

usb_rx_fn usb_rx_hook_get(void)
{
    return s_rx_hook;
}

/*
 * 拔掉上拉再插上，主机才会重新枚举。
 * 不用 Delay_Ms。那个函数占用 SysTick，时基会被拆掉。
 * 循环次数按 USB_HCLK_HZ 估算，够几十毫秒即可。
 */
static void usb_wait_ms(uint32_t ms)
{
    volatile uint32_t spin;
    uint32_t n;

    n = (USB_HCLK_HZ / 4000u) * ms;
    for (spin = 0u; spin < n; spin++)
    {
    }
}

void USBD_SetConfiguration(void)
{
    DEVICE_INFO *info;

    info = &Device_Info;
    if (info->Current_Configuration != 0u)
    {
        bDeviceState = CONFIGURED;
        fSuspendEnabled = TRUE;
        usb_log_configured((uint8_t)info->Current_Configuration);
    }
}

void USBD_SetDeviceAddress(void)
{
    bDeviceState = ADDRESSED;
    usb_log_address((uint8_t)pInformation->USBwValue0);
}

void USBD_SetDeviceFeature(void)
{
}

void USBD_ClearFeature(void)
{
}

void USBD_Status_In(void)
{
    if (pInformation->USBbRequest == CDC_SET_LINE_CODING)
    {
        usb_log_line(s_line);
    }
}

void USBD_Status_Out(void)
{
}

void USBD_init(void)
{
    uint8_t i;

    pInformation->Current_Configuration = 0;
    PowerOn();
    for (i = 0u; i < 8u; i++)
    {
        _SetENDPOINT(i, _GetENDPOINT(i) & 0x7F7F & EPREG_MASK);
    }
    _SetISTR((uint16_t)0x00FF);
    USB_SIL_Init();
    bDeviceState = UNCONNECTED;

    USB_Port_Set(DISABLE, DISABLE);
    usb_wait_ms(USB_DISC_MS);
#if USB_USE_INTERNAL_PU
    USB_Port_Set(ENABLE, ENABLE);
#else
    USB_Port_Set(ENABLE, DISABLE);
#endif
    /* 退出掉电会把中断屏蔽清掉。上拉打开后再写一次。 */
    wInterrupt_Mask = IMR_MSK;
    _SetCNTR(wInterrupt_Mask);
    /* 配置完成前不允许挂起。挂起里的恢复会把总线驱动住，枚举就停了。 */
    fSuspendEnabled = FALSE;
    usb_log_attach();
}

void USBD_Reset(void)
{
    pInformation->Current_Configuration = 0;
    pInformation->Current_Feature = USBD_ConfigDescriptor[7];
    pInformation->Current_Interface = 0;

    SetBTABLE(BTABLE_ADDRESS);

    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    Clear_Status_Out(ENDP0);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxValid(ENDP0);
    _ClearDTOG_RX(ENDP0);
    _ClearDTOG_TX(ENDP0);

    SetEPType(ENDP1, EP_INTERRUPT);
    SetEPTxStatus(ENDP1, EP_TX_NAK);
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);
    SetEPRxStatus(ENDP1, EP_RX_DIS);
    _ClearDTOG_TX(ENDP1);
    _ClearDTOG_RX(ENDP1);

    SetEPType(ENDP2, EP_BULK);
    SetEPTxStatus(ENDP2, EP_TX_DIS);
    SetEPRxAddr(ENDP2, ENDP2_RXADDR);
    SetEPRxCount(ENDP2, DEF_USBD_MAX_PACK_SIZE);
    SetEPRxStatus(ENDP2, EP_RX_VALID);
    _ClearDTOG_RX(ENDP2);
    _ClearDTOG_TX(ENDP2);

    SetEPType(ENDP3, EP_BULK);
    SetEPTxStatus(ENDP3, EP_TX_NAK);
    SetEPTxAddr(ENDP3, ENDP3_TXADDR);
    SetEPRxStatus(ENDP3, EP_RX_DIS);
    _ClearDTOG_TX(ENDP3);
    _ClearDTOG_RX(ENDP3);

    SetDeviceAddress(0);
    USBD_Endp3_Busy = 0;
    bDeviceState = ATTACHED;
}

uint8_t *USBD_GetDeviceDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

uint8_t *USBD_GetConfigDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

uint8_t *USBD_GetStringDescriptor(uint16_t Length)
{
    uint8_t index;

    index = pInformation->USBwValue0;
    if (index >= 4u)
    {
        return NULL;
    }
    return Standard_GetDescriptorData(Length, &String_Descriptor[index]);
}

RESULT USBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
    if (AlternateSetting > 0u)
    {
        return USB_UNSUPPORT;
    }
    if (Interface > 1u)
    {
        return USB_UNSUPPORT;
    }
    return USB_SUCCESS;
}

static uint8_t *usb_line_copy(uint16_t Length)
{
    if (Length == 0u)
    {
        pInformation->Ctrl_Info.Usb_wLength = 7u;
        return NULL;
    }
    return s_line;
}

RESULT USBD_Data_Setup(uint8_t RequestNo)
{
    uint8_t *(*copy)(uint16_t);
    uint32_t request;

    (void)RequestNo;
    copy = NULL;
    request = pInformation->USBbRequest;
    if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
    {
        return USB_UNSUPPORT;
    }
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
        if (request == CDC_GET_LINE_CODING || request == CDC_SET_LINE_CODING)
        {
            copy = usb_line_copy;
        }
        else
        {
            return USB_UNSUPPORT;
        }
    }
    if (copy == NULL)
    {
        return USB_UNSUPPORT;
    }
    pInformation->Ctrl_Info.CopyData = copy;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (void)copy(0);
    return USB_SUCCESS;
}

RESULT USBD_NoData_Setup(uint8_t RequestNo)
{
    uint32_t request;

    (void)RequestNo;
    request = pInformation->USBbRequest;
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
        if (request != CDC_SET_LINE_CTLSTE && request != CDC_SEND_BREAK)
        {
            return USB_UNSUPPORT;
        }
    }
    return USB_SUCCESS;
}
