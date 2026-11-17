#include "ota_boot.h"
#include "config.h"
#include "crc.h"
#include <stddef.h>

#if (APP_SLOT == 0)
#define IMG_LINK_OFS    MEM_SLOT_A_OFS
#elif (APP_SLOT == 1)
#define IMG_LINK_OFS    MEM_SLOT_B_OFS
#endif

#define OTA_FLAG_ADDR   (MEM_FLASH_PHY_BASE + MEM_FLAG_OFS)
#define OTA_FLAG_NUM    (MEM_FLAG_SIZE / sizeof(ota_flag_t))

extern char _img_end[];

typedef char ota_flag_count_check[((MEM_FLAG_SIZE % sizeof(ota_flag_t)) == 0u) ? 1 : -1];

/* 没有别的代码引用它。链接脚本 KEEP 这一段，避免被丢掉。 */
const img_hdr_t g_img_hdr __attribute__((section(".img_hdr"), used)) =
{
    MEM_IMG_MAGIC,
    IMG_LINK_OFS,
    FW_VERSION,
    (uint32_t)_img_end
};

/* 32 条里，魔术字对且前 60 字节 CRC 对的，取 seq 最大。 */
static result_t ota_flag_load(ota_flag_t *out)
{
    const ota_flag_t *rec;
    const ota_flag_t *best;
    uint32_t i;
    uint32_t crc;

    if (out == 0)
    {
        return RES_PARAM;
    }

    best = 0;
    rec = (const ota_flag_t *)OTA_FLAG_ADDR;
    for (i = 0u; i < OTA_FLAG_NUM; i++)
    {
        if (rec[i].magic != OTA_FLAG_MAGIC)
        {
            continue;
        }
        crc = crc_sw_driver.pf_crc32((const uint8_t *)&rec[i], (uint32_t)offsetof(ota_flag_t, crc));
        if (crc != rec[i].crc)
        {
            continue;
        }
        if ((best == 0) || (rec[i].seq > best->seq))
        {
            best = &rec[i];
        }
    }

    if (best == 0)
    {
        return RES_STATE;
    }

    *out = *best;
    return RES_OK;
}

const ota_boot_driver_t ota_boot =
{
    .pf_load = ota_flag_load
};
