/*
 * drv_uart.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USART1。接收中断把字节推进环形缓冲，发送中断从队列把字节写出去。
 *
 * 约定：
 *   1. 接收中断是接收环的唯一生产者。上层只能读。
 *   2. 接收中断只把字节推进环。发送中断只从发送队列取一个字节写进数据寄存器。不解析，不打印，不等待。
 *   3. pf_putc 把字节放进发送队列就返回。队列满返回 RES_BUSY，这一字节没有入队。9 位数据时只写出低 8 位。
 *   4. 不要再调用 Debug 里的 USART_Printf_Init 或 Delay_Us。前者会重配这只串口，后者会改掉时基。
 *   5. 改波特率、数据位、停止位、校验位走 pf_line_set，一次写齐，返回前已经生效。
 *      7 位必须带校验，9 位不能再带校验。这颗芯片的字长只有 8 位和 9 位，校验占最高一位。
 *      已经在环里的字节保留。发送队列里还没写进移位寄存器的字节丢掉。
 *      改参数时数据寄存器里还没读走的那一字节丢掉。
 *   6. 9 位数据在环里占两个字节：先低 8 位，再高字节，高字节只有 bit0 是第 9 位数据。
 */
#ifndef FW_APP_03_DRIVER_UART_DRV_UART_H_
#define FW_APP_03_DRIVER_UART_DRV_UART_H_

#include <stdint.h>
#include "platform.h"
#include "ring_buffer.h"

/* 数据位是有效数据，不含校验位。 */
typedef enum
{
    UART_DATA_BITS_7 = 7,
    UART_DATA_BITS_8 = 8,
    UART_DATA_BITS_9 = 9
} uart_data_bits_t;

typedef enum
{
    UART_STOP_BITS_1 = 0,     /* 1 位 */
    UART_STOP_BITS_0_5,       /* 0.5 位 */
    UART_STOP_BITS_2,         /* 2 位 */
    UART_STOP_BITS_1_5        /* 1.5 位 */
} uart_stop_bits_t;

typedef enum
{
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} uart_parity_t;

/* 四个参数一次交给驱动。上层不要拆成四次改。 */
typedef struct
{
    uint32_t         baud;        /* 波特率，单位 bps。0 非法 */
    uart_data_bits_t data_bits;   /* 7、8 或 9 */
    uart_stop_bits_t stop_bits;
    uart_parity_t    parity;
} uart_line_t;

typedef struct
{
    /*
     * pf_init：配置引脚、串口和中断，并清空接收环和发送队列。
     * 返回 RES_OK。时基已经由板级读好，这里不再改时钟。
     */
    result_t (*pf_init)(void);
    /*
     * pf_rx_ring：交出接收环。
     * 生产者是本驱动的中断。消费者在主循环里用 rb_sw_driver 读。
     * 同一时刻只能有一个消费者。现在是 USB 协议任务。
     */
    rb_t    *(*pf_rx_ring)(void);
    /*
     * pf_putc：把一个字节放进发送队列，并打开发送中断。
     * 形参：ch —— 要发送的字节。
     * 返回：RES_OK 已经入队；RES_BUSY 队列满，这一字节没有入队；RES_STATE 还没初始化。
     */
    result_t (*pf_putc)(uint8_t ch);
    /*
     * pf_tx_free：发送队列还能放多少字节。
     * 上层先看空位，再从接收环拿走，避免拿走了却放不进队列。
     */
    uint16_t (*pf_tx_free)(void);
    /*
     * pf_line_set：立刻改线路参数。
     * 形参：line —— 波特率、数据位、停止位、校验位。不能为 NULL。
     * 返回：
     *   RES_OK     寄存器已经按新参数在跑；
     *   RES_PARAM  指针空、波特率这颗芯片装不下，或数据位和校验的组合非法；
     *   RES_STATE  还没调用过 pf_init，或总线时钟还没读到。
     * 参数不合法时，原来的线路保持不动。
     */
    result_t (*pf_line_set)(const uart_line_t *line);
    /*
     * pf_line_get：读出当前正在用的线路参数。
     * 形参：line —— 输出。不能为 NULL。
     * 返回：RES_OK / RES_PARAM / RES_STATE。
     */
    result_t (*pf_line_get)(uart_line_t *line);
} drv_uart_driver_t;

extern const drv_uart_driver_t drv_uart;

#endif /* FW_APP_03_DRIVER_UART_DRV_UART_H_ */
