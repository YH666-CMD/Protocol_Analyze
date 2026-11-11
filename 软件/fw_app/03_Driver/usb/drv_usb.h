/*
 * drv_usb.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 全速设备。枚举成描述符里写的虚拟串口。
 *
 * 约定：
 *   1. 枚举在 USB 中断里完成。pf_init 只打开时钟、上拉和中断。
 *   2. 主频、厂商号、包长、上拉和打印开关在 usb_cfg.h。移植先改那个文件。
 *   3. 主机写下的线路参数存在驱动里，不改 USART1。以后上层再决定要不要套用。
 *   4. pf_tx 一次最多 USB_EP_MPS 字节。没枚举完或上一包还没发完就返回错误。
 *   5. 接收不在本驱动里建环。pf_on_rx_set 的回调在中断里执行，只允许把字节拷走。
 *      上层把回调接到 USB 接收环。解析和组帧不在中断里做。
 *   6. 发送是一包一包的。pf_tx_ready 告诉上层这一拍最多能交多少字节。
 *      上层先看这个值，再从串口环取不超过本帧能装下的字节，组帧后调用 pf_tx。
 *   7. 打印走 usb_log.h。默认不编译。登记的函数不能睡眠。
 *   8. 不调用 Delay_Us / Delay_Ms，也不调用 USART_Printf_Init。
 *   9. 空闲时不要让 CPU 进入 WFI。USB 写分组缓冲时要 CPU 醒着，否则这一包会被丢掉。
 */
#ifndef FW_APP_03_DRIVER_USB_DRV_USB_H_
#define FW_APP_03_DRIVER_USB_DRV_USB_H_

#include <stdint.h>
#include "platform.h"
#include "usb_cfg.h"

/* 数值和原厂 DEVICE_STATE 一致。主机认完设备后是 USB_STATE_CONFIGURED。 */
typedef enum
{
    USB_STATE_UNCONNECTED = 0,
    USB_STATE_ATTACHED,
    USB_STATE_POWERED,
    USB_STATE_SUSPENDED,
    USB_STATE_ADDRESSED,
    USB_STATE_CONFIGURED
} usb_state_t;

/* 主机 CDC 请求里的线路参数。停止位 0/1/2 表示 1、1.5、2 位。校验 0/1/2 表示无、奇、偶。 */
typedef struct
{
    uint32_t baud;
    uint8_t  stop_bits;
    uint8_t  parity;
    uint8_t  data_bits;
} usb_cdc_line_t;

typedef void (*usb_rx_fn)(const uint8_t *data, uint16_t len);

typedef struct
{
    /*
     * pf_init：打开 USB 时钟和上拉，开始枚举。
     * 返回：RES_OK。主频不在 usb_cfg.h 允许的三档时，编译直接失败。
     * 重复调用直接返回 RES_OK，不会再次断开上拉。
     */
    result_t (*pf_init)(void);
    /*
     * pf_state：当前枚举走到哪一步。
     * 返回 USB_STATE_CONFIGURED 表示主机已经完成配置。
     */
    usb_state_t (*pf_state)(void);
    /*
     * pf_line_get：读主机最后一次写下的线路参数。
     * 形参：line —— 输出，不能为 NULL。
     * 返回：RES_OK / RES_PARAM / RES_STATE。还没下发时读到的是默认 115200 8N1。
     */
    result_t (*pf_line_get)(usb_cdc_line_t *line);
    /*
     * pf_tx：向主机发一包，长度 1 到 USB_EP_MPS。
     * 返回：RES_OK 已经交给端点；RES_BUSY 上一包还没发完；RES_PARAM 参数不对；
     *       RES_STATE 还没初始化，或主机还没配置完。
     */
    result_t (*pf_tx)(const uint8_t *data, uint16_t len);
    /*
     * pf_tx_ready：这一拍还能发给主机多少字节。
     * 返回 0：还没初始化、主机还没配置完，或上一包还没发完。
     * 返回 USB_EP_MPS：可以立刻交一包，长度不能超过这个数。
     */
    uint16_t (*pf_tx_ready)(void);
    /*
     * pf_on_rx_set：登记主机下发数据的回调。传 NULL 则收到后丢掉。
     * 返回：RES_OK。回调在中断里执行，返回前必须把字节拷走。
     */
    result_t (*pf_on_rx_set)(usb_rx_fn fn);
} drv_usb_driver_t;

extern const drv_usb_driver_t drv_usb;

#endif /* FW_APP_03_DRIVER_USB_DRV_USB_H_ */
