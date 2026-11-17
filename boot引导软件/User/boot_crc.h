/*
 * boot_crc.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: Boot 用的 CRC-32。和 zlib 相同，只算标志记录。
 */
#ifndef USER_BOOT_CRC_H_
#define USER_BOOT_CRC_H_

#include <stdint.h>

uint32_t boot_crc32(const uint8_t *data, uint32_t len);

#endif /* USER_BOOT_CRC_H_ */
