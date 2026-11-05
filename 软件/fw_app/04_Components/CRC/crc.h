/*
 * crc.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 常用 CRC 软件接口。
 */
#ifndef FW_APP_04_COMPONENTS_CRC_CRC_H_
#define FW_APP_04_COMPONENTS_CRC_CRC_H_

#include <stdint.h>

typedef struct
{
    /* CRC-8/SMBUS 简单校验*/
    uint8_t  (*pf_crc8)(const uint8_t *data, uint32_t len);
    /* CRC-8/MAXIM 常用于美信芯片温度传感器等*/
    uint8_t  (*pf_crc8_maxim)(const uint8_t *data, uint32_t len);
    /* CRC-16/MODBUS 工业控制常用Modbus RTU协议*/
    uint16_t (*pf_crc16_modbus)(const uint8_t *data, uint32_t len);
    /* CRC-16/CCITT-FALSE 通信领域常用X.25、蓝牙、ZigBee*/
    uint16_t (*pf_crc16_ccitt)(const uint8_t *data, uint32_t len);
    /* CRC-16/XMODEM 常用于传输文件,XMODEM协议*/
    uint16_t (*pf_crc16_xmodem)(const uint8_t *data, uint32_t len);
    /* CRC-32/ISO-HDLC 最经典，常用以太网、ZIP压缩、PNG图片校验*/
    uint32_t (*pf_crc32)(const uint8_t *data, uint32_t len);
    /* CRC-32/MPEG-2 视频流传输、部分固件校验*/
    uint32_t (*pf_crc32_mpeg2)(const uint8_t *data, uint32_t len);
} crc_driver_interface_t;

extern const crc_driver_interface_t crc_sw_driver;

#endif /* FW_APP_04_COMPONENTS_CRC_CRC_H_ */
