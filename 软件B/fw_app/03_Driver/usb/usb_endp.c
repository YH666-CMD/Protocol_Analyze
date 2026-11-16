/*
 * usb_endp.c
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 非控制端点。端点 2 收主机的数据，端点 3 发给主机。
 *
 * 端点 2 收完必须重新打开，不然后面的包会堵住。
 * 没有登记接收回调时，字节在回调返回前就丢掉，不在这里建循环缓冲。
 * 端点 3 发完由中断清掉忙标志。pf_tx_ready 看的就是这个标志。
 */
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "usb_prop.h"
#include "usb_log.h"
#include "drv_usb.h"

uint8_t USBD_Endp3_Busy;

/* 端点 1 是中断 IN，用来向主机报串口状态。现在没有状态要报，函数体留空。符号必须在，原厂表会调用它。 */
void EP1_IN_Callback(void)
{
}

/* 端点 2 收到一包。先拷出分组缓冲，再打开下一次接收。回调返回前必须把字节拿走。 */
void EP2_OUT_Callback(void)
{
    uint8_t buf[DEF_USBD_MAX_PACK_SIZE];
    uint16_t len;
    usb_rx_fn hook;

    len = (uint16_t)GetEPRxCount(EP2_OUT & 0x7F);
    if (len > DEF_USBD_MAX_PACK_SIZE)
    {
        len = DEF_USBD_MAX_PACK_SIZE;
    }
    PMAToUserBufferCopy(buf, GetEPRxAddr(EP2_OUT & 0x7F), len);
    usb_log_rx(len);
    hook = usb_rx_hook_get();
    if (hook != NULL && len != 0u)
    {
        hook(buf, len);
    }
    SetEPRxValid(ENDP2);
}

/* 端点 3 的一包已经被主机取走。清掉忙标志，上层才能交下一包。 */
void EP3_IN_Callback(void)
{
    USBD_Endp3_Busy = 0u;
}

uint16_t usb_ep3_room(void)
{
    if (USBD_Endp3_Busy != 0u)
    {
        return 0u;
    }
    return (uint16_t)DEF_USBD_MAX_PACK_SIZE;
}

/* 把一包交给端点 3。只接受端点 3。上一包还没发完时返回失败，不覆盖正在发的数据。 */
uint8_t USBD_ENDPx_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len)
{
    if (endp != ENDP3 || pbuf == NULL)
    {
        return USB_ERROR;
    }
    if (USBD_Endp3_Busy != 0u)
    {
        return USB_ERROR;
    }
    USB_SIL_Write(EP3_IN, pbuf, len);
    USBD_Endp3_Busy = 1u;
    SetEPTxStatus(ENDP3, EP_TX_VALID);
    return USB_SUCCESS;
}
