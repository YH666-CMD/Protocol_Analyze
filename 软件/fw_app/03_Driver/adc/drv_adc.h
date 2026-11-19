/*
 * drv_adc.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: ADC1 三路扫描。样本只放在一块乒乓缓冲里。
 *
 * 约定：
 *   1. pf_start 可以连着调用。规则组 PA0、PA1、PA2，连续转换，DMA 循环。
 *   2. pf_stop 可以连着调用。停 DMA1 通道 1 和 ADC，不关 DMA 总时钟，不碰通道 5。
 *   3. 不再另开 1024 样本的数组。DMA 的内存就是这块 pp_t。
 *   4. 中断里只发布半区。校准和裁剪到 0～4095 在 pf_take 里做。
 *   5. 引脚只从 pin_map.h 取。通道号只从 dma_config.h 取。
 */
#ifndef FW_APP_03_DRIVER_ADC_DRV_ADC_H_
#define FW_APP_03_DRIVER_ADC_DRV_ADC_H_

#include "platform.h"
#include "pingpong_buf.h"

#define ADC_CH_NUM   3u

typedef struct
{
    /*
     * pf_start：打开三路采样。已经开着就直接返回。
     * 返回 RES_OK。校准转不完返回 RES_OVERRUN，硬件会停回关闭。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 ADC 和它的 DMA。可以连着调用。
     */
    void (*pf_stop)(void);
    /*
     * pf_take：取走刚填满的半区，加上校准值后裁到 0～4095。
     * out 至少 PP_HALF_SIZE 个样本，按通道排开：out[通道号 * 轮数 + 轮号]。
     * 没有新半区，或还没打开，返回 false。
     */
    bool (*pf_take)(pp_sample_t *out, uint16_t *rounds);
} drv_adc_driver_t;

extern const drv_adc_driver_t drv_adc;

#endif /* FW_APP_03_DRIVER_ADC_DRV_ADC_H_ */
