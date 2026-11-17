#include "ota_boot.h"
#include "config.h"
#include "crc.h"
#include "bsp_flash.h"
#include <stddef.h>

#if (APP_SLOT == 0)
#define IMG_LINK_OFS    MEM_SLOT_A_OFS
#elif (APP_SLOT == 1)
#define IMG_LINK_OFS    MEM_SLOT_B_OFS
#endif

#define OTA_FLAG_ADDR       (MEM_FLASH_PHY_BASE + MEM_FLAG_OFS)
#define OTA_FLAG_NUM        (MEM_FLAG_SIZE / sizeof(ota_flag_t))
#define OTA_FLAG_PER_PAGE   (MEM_FLASH_PAGE / sizeof(ota_flag_t))

extern char _img_end[];

typedef char ota_flag_count_check[((MEM_FLAG_SIZE % sizeof(ota_flag_t)) == 0u) ? 1 : -1];
typedef char ota_flag_page_check[((MEM_FLASH_PAGE % sizeof(ota_flag_t)) == 0u) ? 1 : -1];

/* 没有别的代码引用它。链接脚本 KEEP 这一段，避免被丢掉。 */
const img_hdr_t g_img_hdr __attribute__((section(".img_hdr"), used)) =
{
    MEM_IMG_MAGIC,
    IMG_LINK_OFS,
    FW_VERSION,
    (uint32_t)_img_end
};

static uint32_t ota_flag_crc(const ota_flag_t *rec)
{
    return crc_sw_driver.pf_crc32((const uint8_t *)rec, (uint32_t)offsetof(ota_flag_t, crc));
}

/* 没有有效记录时返回 OTA_FLAG_NUM。 */
static uint32_t ota_flag_latest(void)
{
    const ota_flag_t *rec;
    uint32_t i;
    uint32_t best;

    best = OTA_FLAG_NUM;
    rec = (const ota_flag_t *)OTA_FLAG_ADDR;
    for (i = 0u; i < OTA_FLAG_NUM; i++)
    {
        if (rec[i].magic != OTA_FLAG_MAGIC)
        {
            continue;
        }
        if (ota_flag_crc(&rec[i]) != rec[i].crc)
        {
            continue;
        }
        if ((best == OTA_FLAG_NUM) || (rec[i].seq > rec[best].seq))
        {
            best = i;
        }
    }
    return best;
}

static result_t ota_flag_load(ota_flag_t *out)
{
    uint32_t latest;

    if (out == 0)
    {
        return RES_PARAM;
    }
    latest = ota_flag_latest();
    if (latest >= OTA_FLAG_NUM)
    {
        return RES_STATE;
    }
    *out = ((const ota_flag_t *)OTA_FLAG_ADDR)[latest];
    return RES_OK;
}

/* crc 在结构体最后。半字写从低地址往高地址写，所以它最后落地。 */
static result_t ota_flag_save(const ota_flag_t *in)
{
    const ota_flag_t *slot;
    ota_flag_t rec;
    uint32_t latest;
    uint32_t index;
    uint32_t n;
    uint32_t addr;
    result_t rc;

    if (in == 0)
    {
        return RES_PARAM;
    }

    slot = (const ota_flag_t *)OTA_FLAG_ADDR;
    latest = ota_flag_latest();
    index = 0u;
    if (latest < OTA_FLAG_NUM)
    {
        index = latest + 1u;
        if (index >= OTA_FLAG_NUM)
        {
            index = 0u;
        }
    }

    for (n = 0u; n < OTA_FLAG_NUM; n++)
    {
        if (index == latest)
        {
            return RES_STATE;
        }
        addr = OTA_FLAG_ADDR + (index * sizeof(ota_flag_t));
        if ((index % OTA_FLAG_PER_PAGE) == 0u)
        {
            /* 这一页里还有当前记录，不能擦。 */
            if ((latest < OTA_FLAG_NUM) &&
                ((latest / OTA_FLAG_PER_PAGE) == (index / OTA_FLAG_PER_PAGE)))
            {
                return RES_STATE;
            }
            rc = bsp_flash.pf_erase(addr, MEM_FLASH_PAGE);
            if (rc != RES_OK)
            {
                return rc;
            }
        }
        if (bsp_flash.pf_is_blank(addr, sizeof(ota_flag_t)) != 0u)
        {
            break;
        }
        index++;
        if (index >= OTA_FLAG_NUM)
        {
            index = 0u;
        }
    }
    if (n >= OTA_FLAG_NUM)
    {
        return RES_STATE;
    }

    rec = *in;
    rec.magic = OTA_FLAG_MAGIC;
    if (latest < OTA_FLAG_NUM)
    {
        rec.seq = slot[latest].seq + 1u;
    }
    else
    {
        rec.seq = 1u;
    }
    rec.crc = ota_flag_crc(&rec);
    return bsp_flash.pf_write(addr, (const uint8_t *)&rec, sizeof(rec));
}

void ota_flag_factory(void)
{
#if (APP_SLOT == 0)
    ota_flag_t rec;
    uint32_t addr;
    uint32_t size;
    uint32_t i;

    if (ota_flag_latest() < OTA_FLAG_NUM)
    {
        return;
    }
    addr = MEM_FLASH_RUN_BASE + IMG_LINK_OFS;
    if ((uint32_t)_img_end <= addr)
    {
        return;
    }
    size = (uint32_t)_img_end - addr;
    if (size > MEM_SLOT_SIZE)
    {
        return;
    }
    for (i = 0u; i < (sizeof(rec) / 4u); i++)
    {
        ((uint32_t *)&rec)[i] = 0u;
    }
    rec.run_slot = OTA_SLOT_A;
    rec.trial_slot = OTA_SLOT_NONE;
    rec.slot[OTA_SLOT_A].state = OTA_ST_GOOD;
    rec.slot[OTA_SLOT_A].size = size;
    rec.slot[OTA_SLOT_A].crc32 = crc_sw_driver.pf_crc32((const uint8_t *)addr, size);
    rec.slot[OTA_SLOT_A].version = FW_VERSION;
    (void)ota_flag_save(&rec);
#endif
}

const ota_boot_driver_t ota_boot =
{
    .pf_load = ota_flag_load,
    .pf_save = ota_flag_save
};
