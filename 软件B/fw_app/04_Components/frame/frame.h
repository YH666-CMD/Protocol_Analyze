/*
 * frame.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 通信帧。组包和拆包都在这里，不碰 USB 寄存器，也不碰串口。
 *
 * 约定：
 *   1. 所有经 USB 上下行的业务都走本帧。串口、PWM、SPI、I2C 的数据先组帧再交给 USB。
 *      ADC 先走乒乓缓冲，取出半区后再组帧。
 *   2. 帧格式和第 2 字节起的长度字段，与环的 pf_rb_sync_and_peek 一致：
 *        [同步字][整帧长度][指令][数据 0..N][CRC16 低字节][CRC16 高字节]
 *      整帧长度含同步字到 CRC，最小 FRAME_MIN，最大 FRAME_MAX。
 *   3. CRC 是 CRC-16/CCITT-FALSE，覆盖同步字到数据末尾，不含 CRC 自己。
 *   4. 一帧不超过 FRAME_MAX，正好装进一包全速批量传输。超过就由调用方拆成多帧。
 *   5. 本文件不包含 USB、串口头。指令号在这里统一定义，处理函数在业务层登记。
 *   6. 主机要设备做事，看指令字节。关外设、开采样走 FRAME_CMD_CTRL，
 *      数据区第 1 字节是模块，第 2 字节是动作，后面才是该动作的参数。
 *   7. 波特率不下发具体数字。UART_LINE 数据区第 1 字节是波特率功能码。
 *      只发这 1 字节就只改波特率。发满 4 字节再带停止位、校验、数据位。
 *      新波特率在功能码表加一行，不改解析流程。
 *   8. 设备上传和主机下发用同一套指令号。上传由业务层调用上报接口。
 */
#ifndef FW_APP_04_COMPONENTS_FRAME_FRAME_H_
#define FW_APP_04_COMPONENTS_FRAME_FRAME_H_

#include <stdint.h>
#include "platform.h"

#define FRAME_SOF           0xA5u
#define FRAME_OH            5u
#define FRAME_MAX           64u
#define FRAME_MIN           FRAME_OH
#define FRAME_DATA_MAX      (FRAME_MAX - FRAME_OH)

#if (FRAME_MAX < FRAME_MIN) || (FRAME_MAX > 255u)
#error "FRAME_MAX must fit in one length byte and be at least FRAME_MIN"
#endif

/* 设备发给主机。0x10 一段留给串口，0x20 PWM，0x30 ADC，0x40 SPI，0x50 I2C。 */
#define FRAME_CMD_ACK       0x01u
#define FRAME_CMD_NACK      0x02u
#define FRAME_CMD_UART_RX   0x10u
#define FRAME_CMD_PWM_IN    0x20u
#define FRAME_CMD_ADC       0x30u
#define FRAME_CMD_SPI       0x40u
#define FRAME_CMD_I2C       0x50u

/*
 * 主机发给设备。高位置 1。
 * CTRL 是控制面：关外设、开外设、复位、开始采、停止采。
 * 数据流仍用 UART_TX、PWM_OUT 这些指令，不要把采样数据塞进 CTRL。
 */
#define FRAME_CMD_PING      0x80u
#define FRAME_CMD_CTRL      0x81u
#define FRAME_CMD_UART_TX   0x90u
#define FRAME_CMD_UART_LINE 0x91u

/*
 * 波特率功能码。主机只发这个码，设备查表得到波特率。
 * 以后要加新波特率，在这里加一个码，再在功能码表加一行。
 */
#define FRAME_BAUD_115200   1u
#define FRAME_BAUD_9600     2u
#define FRAME_BAUD_19200    3u
#define FRAME_BAUD_38400    4u
#define FRAME_BAUD_57600    5u
#define FRAME_BAUD_230400   6u
#define FRAME_BAUD_460800   7u
#define FRAME_BAUD_921600   8u

/* UART_LINE 第 2、3、4 字节。和波特率功能码一样，主机发码，不发寄存器值。 */
#define FRAME_STOP_1        0u
#define FRAME_STOP_1_5      1u
#define FRAME_STOP_2        2u
#define FRAME_PAR_NONE      0u
#define FRAME_PAR_ODD       1u
#define FRAME_PAR_EVEN      2u
#define FRAME_BITS_7        7u
#define FRAME_BITS_8        8u
#define FRAME_BITS_9        9u
#define FRAME_CMD_PWM_OUT   0xA0u
#define FRAME_CMD_ADC_CFG   0xB0u
#define FRAME_CMD_SPI_CFG   0xC0u
#define FRAME_CMD_I2C_CFG   0xD0u

/* CTRL 数据区第 1 字节：对哪个模块动手。 */
#define FRAME_MOD_SYS       0x00u
#define FRAME_MOD_UART      0x01u
#define FRAME_MOD_PWM       0x02u
#define FRAME_MOD_ADC       0x03u
#define FRAME_MOD_SPI       0x04u
#define FRAME_MOD_I2C       0x05u

/* CTRL 数据区第 2 字节：做什么。后面的字节由该模块自己解释。 */
#define FRAME_OP_OFF        0x00u
#define FRAME_OP_ON         0x01u
#define FRAME_OP_RST        0x02u
#define FRAME_OP_START      0x03u
#define FRAME_OP_STOP       0x04u

typedef struct
{
    /*
     * pf_pack：把指令和数据封成一帧。
     * 形参：
     *   cmd      —— 指令号；
     *   data     —— 数据。data_len 为 0 时允许 NULL；
     *   data_len —— 数据字节数，不能超过 FRAME_DATA_MAX；
     *   out      —— 输出缓冲；
     *   out_cap  —— 输出容量，必须不小于整帧长度；
     *   out_len  —— 输出整帧长度。
     * 返回：RES_OK / RES_PARAM。
     */
    result_t (*pf_pack)(uint8_t cmd, const uint8_t *data, uint16_t data_len,
                        uint8_t *out, uint16_t out_cap, uint16_t *out_len);
    /*
     * pf_unpack：核对同步字、长度和 CRC，再交出指令和数据。
     * 形参：
     *   in       —— 一整帧；
     *   in_len   —— 缓冲区里的字节数，必须等于帧里的长度字段；
     *   cmd      —— 输出指令号；
     *   data     —— 输出数据指针，指向 in 内部，只在 in 有效期间能用；
     *   data_len —— 输出数据字节数，可以为 0。
     * 返回：RES_OK / RES_PARAM。
     */
    result_t (*pf_unpack)(const uint8_t *in, uint16_t in_len,
                          uint8_t *cmd, const uint8_t **data, uint16_t *data_len);
} frame_driver_t;

extern const frame_driver_t frame_sw_driver;

#endif /* FW_APP_04_COMPONENTS_FRAME_FRAME_H_ */
