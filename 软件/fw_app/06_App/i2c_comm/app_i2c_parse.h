/*
 * app_i2c_parse.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 0xD0 的一帧。主机不拉通知脚，从机有数据才拉低。
 *
 * 约定：
 *   1. 第 1 字节 0 主机，1 从机。第 2 字节是 7 位地址。
 *   2. 第 3 字节：0 只切换，1 扫描，2 发送，3 读取。读取时第 4 字节是长度。
 *   3. 上报不在这里组帧。USB 任务从驱动把 0x50 拿走。
 */
#ifndef FW_APP_06_APP_I2C_COMM_APP_I2C_PARSE_H_
#define FW_APP_06_APP_I2C_COMM_APP_I2C_PARSE_H_

#include "platform.h"

result_t app_i2c_cfg(const uint8_t *data, uint16_t len);

#endif /* FW_APP_06_APP_I2C_COMM_APP_I2C_PARSE_H_ */
