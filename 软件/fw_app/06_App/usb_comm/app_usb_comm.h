/*
 * app_usb_comm.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 协议任务。串口环的消费者。下行帧在这里拆开，上行在这里组帧。
 *
 * 约定：
 *   1. 初始化打开 USB，并把接收回调接到本任务的 USB 接收环。
 *   2. 串口接收环只有这一个消费者。打印任务不再读那个环。
 *   3. 主机下发的字节先进 USB 接收环，任务里再按帧拆。USB 中断只拷贝，不解析。
 *   4. 上行先看 USB 还能发多少，再看串口环有多少，再决定这一帧带走多少数据：
 *        环空：不发；
 *        不够一帧数据上限：有多少带多少；
 *        正好：带走这一帧；
 *        超过：只带走本帧装得下的，剩下留在环里。
 *      USB 还没配置完，或上一包没发完：不读环。
 *   5. 设备上传统一走 app_usb_comm_report。PWM、ADC、SPI、I2C 在各自任务里调用它。
 *      ADC 先从乒乓缓冲取出半区，再把样本当数据上报。本任务不读那些缓冲。
 *      返回 RES_BUSY 表示这一拍 USB 发不出去，调用方自己留着数据。
 *   6. 主机下发的控制指令在指令表里登记。加一条就加一行，不用改分发流程。
 *      没登记的指令回 NACK。
 *   7. 关外设、开采样走 FRAME_CMD_CTRL。数据区前两字节是模块号和动作号。
 *      每个动作在控制表加一行。
 *   8. 改波特率走 FRAME_CMD_UART_LINE。数据区第 1 字节是波特率功能码，1 表示 115200。
 *      只发这一字节就只改波特率。新波特率在功能码表加一行。
 */
#ifndef FW_APP_06_APP_USB_COMM_APP_USB_COMM_H_
#define FW_APP_06_APP_USB_COMM_APP_USB_COMM_H_

#include <stdint.h>
#include "sched.h"

result_t app_usb_comm_init(void);
result_t app_usb_comm_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event);

/*
 * app_usb_comm_report：设备主动上传一帧。
 * 形参：cmd —— 指令号，和主机下发用同一套；data —— 数据，长度为 0 时允许 NULL；len —— 数据字节数。
 * 返回：RES_OK 已经交给 USB；RES_BUSY 这一拍发不出去；RES_PARAM 指令装不进一帧。
 */
result_t app_usb_comm_report(uint8_t cmd, const uint8_t *data, uint16_t len);

#endif /* FW_APP_06_APP_USB_COMM_APP_USB_COMM_H_ */
