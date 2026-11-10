/*
 * bsp_wdt.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 看门狗。这一层只负责卡死之后不再喂狗，让芯片复位。
 *
 * WDT_ENABLE 为 0 时不写寄存器。
 * 一轮超过上限时，先走 platform 的 wdt_hang 钩子，再走本层重启前的空钩子。
 * pf_reset 是手动重启。它会先走同一个重启前钩子，再把独立看门狗调到最短超时并等待复位。
 * 调试总开关只挡住上电自动开狗，挡不住这次明确调用。
 * 超时上限在 config.h，本头文件不暴露芯片型号。
 */
#ifndef FW_APP_02_BSP_WDT_BSP_WDT_H_
#define FW_APP_02_BSP_WDT_BSP_WDT_H_

#include <stdint.h>
#include "platform.h"

typedef struct
{
    void     (*pf_start)(void);
    /* 形参：这一轮已经用掉的毫秒。超时返回 RES_OVERRUN，并且这次不喂狗 */
    result_t (*pf_kick)(uint32_t round_ms);
    /* 形参：这一轮已经用掉的毫秒。重启前的专用钩子，现在是空的 */
    result_t (*pf_before_reset)(uint32_t round_ms);
    /* 无参。调用后正常不会返回。看门狗没有把芯片复位时才返回 RES_STATE */
    result_t (*pf_reset)(void);
} bsp_wdt_driver_t;

extern const bsp_wdt_driver_t bsp_wdt;

#endif /* FW_APP_02_BSP_WDT_BSP_WDT_H_ */
