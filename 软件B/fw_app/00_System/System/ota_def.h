/*
 * ota_def.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 镜像头和标志区记录。镜像头 16 字节，放在本面偏移 0x100。
 *                    标志区一条 64 字节，2KB 里循环放 32 条。
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

/* 小端字节 46 4C 47 31。不能等于擦除字 0xE339E339。 */
#define OTA_FLAG_MAGIC  0x31474C46u

#define OTA_SLOT_A      0u
#define OTA_SLOT_B      1u
#define OTA_SLOT_NONE   0xFFu   /* trial_slot：没有待试运行的面 */
#define OTA_TRIAL_MAX   3u

/* 槽状态。不用擦除字节 0x39、0xE3。 */
#define OTA_ST_EMPTY    0u
#define OTA_ST_WRITING  1u      /* 正在写入，不能运行 */
#define OTA_ST_READY    2u      /* 写完且 CRC 对，等试运行 */
#define OTA_ST_GOOD     3u
#define OTA_ST_BAD      4u

typedef struct
{
    uint8_t  state;
    uint8_t  rsv[3];
    uint32_t size;      /* 镜像字节数 */
    uint32_t crc32;     /* 明文镜像的 CRC-32，与 zlib 一致 */
    uint32_t version;
} ota_slot_info_t;

typedef struct
{
    uint32_t magic;
    uint32_t seq;           /* 有效记录里最大的就是当前状态 */
    uint8_t  run_slot;      /* 已确认可以运行的面：OTA_SLOT_A 或 OTA_SLOT_B */
    uint8_t  trial_slot;    /* 待试运行的面，OTA_SLOT_NONE 表示没有 */
    uint8_t  trial_cnt;     /* 试运行已经跳了几次，到 OTA_TRIAL_MAX 就回滚 */
    uint8_t  rsv0;
    ota_slot_info_t slot[2];
    uint32_t rsv[4];
    uint32_t crc;           /* 前面 60 字节的 CRC，最后写 */
} ota_flag_t;

typedef char ota_slot_size_check[(sizeof(ota_slot_info_t) == 16u) ? 1 : -1];
typedef char ota_flag_size_check[(sizeof(ota_flag_t) == 64u) ? 1 : -1];
typedef char ota_flag_magic_check[(OTA_FLAG_MAGIC != 0xE339E339u) ? 1 : -1];

#endif /* FW_APP_00_SYSTEM_SYSTEM_OTA_DEF_H_ */
