/*
 * boot_flash.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 按标志区选出这次要跳的地址。没有记录就选 A 面。
 */
#ifndef USER_BOOT_FLASH_H_
#define USER_BOOT_FLASH_H_

#include <stdint.h>

/* 返回要跳的地址。两面都不可用时返回 0。*seq 是当前记录序号，没有记录为 0。 */
uint32_t boot_choose(uint32_t *seq);

#endif /* USER_BOOT_FLASH_H_ */
