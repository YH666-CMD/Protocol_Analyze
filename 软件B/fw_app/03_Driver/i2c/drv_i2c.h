/*
 * drv_i2c.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: I2C 开关。主从收发留到后面，这里只负责打开标志和停干净。
 *
 * 约定：
 *   1. pf_start 可以连着调用。驱动还没写完时只记下已经打开。
 *   2. pf_stop 可以连着调用。关中断，SCL/SDA 松开，再关 I2C 时钟。不用 DMA。
 *   3. 引脚只从 pin_map.h 取。不要重映射到 PB8/PB9。
 */
#ifndef FW_APP_03_DRIVER_I2C_DRV_I2C_H_
#define FW_APP_03_DRIVER_I2C_DRV_I2C_H_

#include "platform.h"

typedef struct
{
    /*
     * pf_start：记下 I2C 已经打开。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 I2C。可以连着调用。
     */
    void (*pf_stop)(void);
} drv_i2c_driver_t;

extern const drv_i2c_driver_t drv_i2c;

#endif /* FW_APP_03_DRIVER_I2C_DRV_I2C_H_ */
