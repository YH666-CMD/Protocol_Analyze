/*
 * ota_boot.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 读标志区里最新的一条记录。镜像头也在这个组件里。
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
} ota_boot_driver_t;

extern const ota_boot_driver_t ota_boot;

#endif /* FW_APP_04_COMPONENTS_OTA_BOOT_OTA_BOOT_H_ */
