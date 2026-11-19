/*
 * drv_i2c.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: I2C1 主机和从机。通知脚 PB4 只由从机拉动。
 *
 * 约定：
 *   1. 主机不驱动 PB4，只读电平。从机有数据没发完就一直拉低，空了才拉高。
 *   2. 一次收发最多 I2C_IO_MAX 字节。不再开大数组，不用 DMA。
 *   3. 中断里只推进一步、搬 1 字节。组 USB 帧在任务里。
 *   4. 引脚只从 pin_map.h 取。不要重映射到 PB8/PB9，也不动 SWJ。
 */
#ifndef FW_APP_03_DRIVER_I2C_DRV_I2C_H_
#define FW_APP_03_DRIVER_I2C_DRV_I2C_H_

#include "platform.h"
#include <stdbool.h>

#define I2C_IO_MAX        40u
#define I2C_RD_DEFAULT    10u

typedef struct
{
    /*
     * pf_start：打开成主机。已经开着就直接返回。PB4 只做输入。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 I2C。可以连着调用。通知脚松开，不再拉着。
     */
    void (*pf_stop)(void);
    /*
     * pf_role：切主机或从机。slave 为 0 是主机，1 是从机。addr 是 7 位地址。
     * 先关掉再按新角色打开。主机不碰 PB4。
     */
    result_t (*pf_role)(uint8_t slave, uint8_t addr);
    /*
     * pf_speed：0 是 100kHz，1 是 400kHz。已经打开就立刻改时钟。
     * 正在传或者正在扫描时返回 RES_BUSY。
     */
    result_t (*pf_speed)(uint8_t fast);
    /*
     * pf_write：主机把数据写给当前地址。从机把数据放进待读缓冲，并拉低 PB4。
     */
    result_t (*pf_write)(const uint8_t *data, uint16_t len);
    /*
     * pf_read：主机按 n 读一次，并把 n 记成以后跟着通知脚读的长度。
     */
    result_t (*pf_read)(uint8_t n);
    /*
     * pf_scan：主机开始扫 0x08～0x77。结果稍后用 pf_take 拿走。
     */
    result_t (*pf_scan)(void);
    /*
     * pf_poll：任务里调用。通知脚仍为低就再读一段，并接着扫地址。不在这里死等。
     */
    void (*pf_poll)(void);
    /*
     * pf_take：拿走一帧待上报的数据。没有就返回 false。
     * 第 1 字节 0 是收到的数据，1 是扫到的地址。
     */
    bool (*pf_take)(uint8_t *dst, uint16_t cap, uint16_t *n);
} drv_i2c_driver_t;

extern const drv_i2c_driver_t drv_i2c;

#endif /* FW_APP_03_DRIVER_I2C_DRV_I2C_H_ */
