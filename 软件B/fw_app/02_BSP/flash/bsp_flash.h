/*
 * bsp_flash.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 256 字节快速擦、快速页写、半字写。函数留在零等待区。
 *
 * 地址可以是 0x0800xxxx，也可以是相对 0x08000000 的偏移。
 * 只允许写另一面、标志区、参数区、故障快照区、日志区。
 * Boot、当前这一面、慢速代码区、出厂区直接拒绝。
 */
#ifndef FW_APP_02_BSP_FLASH_BSP_FLASH_H_
#define FW_APP_02_BSP_FLASH_BSP_FLASH_H_

#include <stdint.h>
#include "platform.h"

typedef struct
{
    result_t (*pf_erase)(uint32_t addr, uint32_t len);                  /* 256 字节对齐，逐页快速擦 */
    result_t (*pf_write)(uint32_t addr, const uint8_t *buf, uint32_t len); /* 半字写，目标必须已擦 */
    result_t (*pf_write_page)(uint32_t addr, const uint32_t *buf);      /* 写满 256 字节，目标必须已擦 */
    uint8_t  (*pf_is_blank)(uint32_t addr, uint32_t len);               /* 1：这一段全是擦除值 */
} bsp_flash_driver_t;

extern const bsp_flash_driver_t bsp_flash;

#endif /* FW_APP_02_BSP_FLASH_BSP_FLASH_H_ */
