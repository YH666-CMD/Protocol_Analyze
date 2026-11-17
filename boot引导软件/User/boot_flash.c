#include "boot_flash.h"
#include "boot_crc.h"
#include "mem_map.h"
#include "ota_def.h"
#include <stddef.h>

#define BOOT_FLAG_ADDR   (MEM_FLASH_PHY_BASE + MEM_FLAG_OFS)
#define BOOT_FLAG_NUM    (MEM_FLAG_SIZE / sizeof(ota_flag_t))

uint32_t boot_flag_seq(void)
{
    const ota_flag_t *rec;
    uint32_t i;
    uint32_t seq;

    seq = 0u;
    rec = (const ota_flag_t *)BOOT_FLAG_ADDR;
    for (i = 0u; i < BOOT_FLAG_NUM; i++)
    {
        if (rec[i].magic != OTA_FLAG_MAGIC)
        {
            continue;
        }
        if (boot_crc32((const uint8_t *)&rec[i], (uint32_t)offsetof(ota_flag_t, crc)) != rec[i].crc)
        {
            continue;
        }
        if (rec[i].seq > seq)
        {
            seq = rec[i].seq;
        }
    }
    return seq;
}
