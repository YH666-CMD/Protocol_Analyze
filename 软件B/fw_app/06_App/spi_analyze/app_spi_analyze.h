/*
 * app_spi_analyze.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: SPI 主机的一帧配置。打开和关闭仍走控制表，这里不关别的功能。
 *
 * 约定：
 *   1. 0xC0 的数据：模式 0～3，接着 0 只改模式，1 表示后面是要发出的字节。
 *   2. 第一版没有从机。没有单独的角色字节。
 *   3. 收回来的字节不在这里组帧，USB 任务从驱动的环里用 0x40 上报。
 */
#ifndef FW_APP_06_APP_SPI_ANALYZE_APP_SPI_ANALYZE_H_
#define FW_APP_06_APP_SPI_ANALYZE_APP_SPI_ANALYZE_H_

#include "platform.h"

/*
 * app_spi_cfg：处理 0xC0 的数据区。
 * 返回 RES_OK / RES_PARAM / RES_STATE / RES_OVERRUN / RES_NO_MEM。
 */
result_t app_spi_cfg(const uint8_t *data, uint16_t len);

#endif /* FW_APP_06_APP_SPI_ANALYZE_APP_SPI_ANALYZE_H_ */
