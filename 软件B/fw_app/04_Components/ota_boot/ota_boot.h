/*
 * ota_boot.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 标志区记录的读和追加。镜像头也在这个组件里。
 */
#ifndef FW_APP_04_COMPONENTS_OTA_BOOT_OTA_BOOT_H_
#define FW_APP_04_COMPONENTS_OTA_BOOT_OTA_BOOT_H_

#include <stdint.h>
#include "platform.h"
#include "ota_def.h"

typedef struct
{
    /* 读出 seq 最大、且 CRC 对的一条。一条都没有返回 RES_STATE。 */
    result_t (*pf_load)(ota_flag_t *out);
    /* 补上 magic、seq、crc，追加到下一条。下一条在页首就先擦那一页。 */
    result_t (*pf_save)(const ota_flag_t *in);
} ota_boot_driver_t;

extern const ota_boot_driver_t ota_boot;

/* A 面且标志区没有有效记录时，按本镜像写第一条。其它情况直接返回。 */
void ota_flag_factory(void);

#endif /* FW_APP_04_COMPONENTS_OTA_BOOT_OTA_BOOT_H_ */
