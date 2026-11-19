/*
 * drv_spi.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: SPI1 主机。PA4 软件片选，PA5 时钟，PA6 进，PA7 出。
 *
 * 约定：
 *   1. 第一版只做主机。没有从机分支。
 *   2. pf_start 可以连着调用。第一次真正打开时钟和引脚，默认模式 0。
 *   3. pf_stop 可以连着调用。关 SPI 时钟，NSS 松开。不用 DMA，也不开 SPI 中断。
 *   4. 模式 0～3 在 pf_mode 里重新初始化。收发在 USB 任务里轮询，每次有次数上限，不死等。
 *   5. 收回来的字节进环。环满丢新字节，不覆盖还没上报的旧字节。计数在环的 overwrite_count。
 *   6. 引脚只从 pin_map.h 取。
 */
#ifndef FW_APP_03_DRIVER_SPI_DRV_SPI_H_
#define FW_APP_03_DRIVER_SPI_DRV_SPI_H_

#include "platform.h"
#include "ring_buffer.h"

/* 上位机一次最多带 40 字节。驱动也不一次转更多。 */
#define SPI_XFER_MAX   40u

typedef struct
{
    /*
     * pf_start：打开 SPI 主机，模式 0。已经开着就直接返回。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 SPI。可以连着调用。
     */
    void (*pf_stop)(void);
    /*
     * pf_rx_ring：交出接收环。还没打开过就返回 NULL。
     * 生产者和消费者都是 USB 任务，没有 SPI 中断。
     */
    rb_t    *(*pf_rx_ring)(void);
    /*
     * pf_mode：切到模式 0～3，并重新初始化 SPI。
     * 返回：RES_OK；RES_PARAM 模式号不对；RES_STATE 还没打开。
     */
    result_t (*pf_mode)(uint8_t mode);
    /*
     * pf_xfer：NSS 拉低，发出 len 个字节，同时把收回来的字节推进环，再把 NSS 拉高。
     * 返回：RES_OK；RES_PARAM 没有数据或超过 SPI_XFER_MAX；
     *       RES_STATE 还没打开；RES_OVERRUN 这一字节的时钟没转完；
     *       RES_NO_MEM 线上已经转完，但环里放不下，新字节被丢掉。
     */
    result_t (*pf_xfer)(const uint8_t *tx, uint16_t len);
} drv_spi_driver_t;

extern const drv_spi_driver_t drv_spi;

#endif /* FW_APP_03_DRIVER_SPI_DRV_SPI_H_ */
