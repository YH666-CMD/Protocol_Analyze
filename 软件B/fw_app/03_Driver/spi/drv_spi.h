/*
 * drv_spi.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: SPI 开关。收发留到后面，这里只负责打开标志和停干净。
 *
 * 约定：
 *   1. pf_start 可以连着调用。驱动还没写完时只记下已经打开。
 *   2. pf_stop 可以连着调用。关 SPI 时钟，NSS 松开，中断关掉。不用 DMA。
 *   3. 引脚只从 pin_map.h 取。
 */
#ifndef FW_APP_03_DRIVER_SPI_DRV_SPI_H_
#define FW_APP_03_DRIVER_SPI_DRV_SPI_H_

#include "platform.h"

typedef struct
{
    /*
     * pf_start：记下 SPI 已经打开。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 SPI。可以连着调用。
     */
    void (*pf_stop)(void);
} drv_spi_driver_t;

extern const drv_spi_driver_t drv_spi;

#endif /* FW_APP_03_DRIVER_SPI_DRV_SPI_H_ */
