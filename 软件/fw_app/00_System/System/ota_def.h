/*
 * ota_def.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 镜像头。16 字节，放在本面偏移 0x100。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_OTA_DEF_H_
#define FW_APP_00_SYSTEM_SYSTEM_OTA_DEF_H_

#include <stdint.h>

typedef struct
{
    uint32_t magic;    /* MEM_IMG_MAGIC */
    uint32_t link_ofs; /* 本镜像该放在哪一面。A 是 0x800，B 是 0x8000 */
    uint32_t version;  /* FW_VERSION */
    uint32_t img_end;  /* 本面镜像在 Flash 里的结束地址，链接时填 */
} img_hdr_t;

#endif /* FW_APP_00_SYSTEM_SYSTEM_OTA_DEF_H_ */
