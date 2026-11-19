/*
 * drv_adc.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: ADC 开关。采样留到后面，这里只负责打开标志和停干净。
 *
 * 约定：
 *   1. pf_start 可以连着调用。驱动还没写完时只记下已经打开。
 *   2. pf_stop 可以连着调用。停 DMA1 通道 1 和 ADC，不碰通道 5。
 *   3. 引脚只从 pin_map.h 取。通道号只从 dma_config.h 取。
 */
#ifndef FW_APP_03_DRIVER_ADC_DRV_ADC_H_
#define FW_APP_03_DRIVER_ADC_DRV_ADC_H_

#include "platform.h"

typedef struct
{
    /*
     * pf_start：记下 ADC 已经打开。
     * 返回 RES_OK。
     */
    result_t (*pf_start)(void);
    /*
     * pf_stop：关掉 ADC 和它的 DMA。可以连着调用。
     */
    void (*pf_stop)(void);
} drv_adc_driver_t;

extern const drv_adc_driver_t drv_adc;

#endif /* FW_APP_03_DRIVER_ADC_DRV_ADC_H_ */
