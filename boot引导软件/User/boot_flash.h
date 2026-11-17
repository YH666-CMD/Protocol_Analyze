/*
 * boot_flash.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 读标志区里序号最大的一条。没有有效记录时返回 0。
 */
#ifndef USER_BOOT_FLASH_H_
#define USER_BOOT_FLASH_H_

#include <stdint.h>

uint32_t boot_flag_seq(void);

#endif /* USER_BOOT_FLASH_H_ */
