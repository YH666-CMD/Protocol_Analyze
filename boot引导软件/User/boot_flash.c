#include "boot_flash.h"
#include "boot_crc.h"
#include "mem_map.h"
#include "ota_def.h"
#include <stddef.h>

#define BOOT_FLAG_ADDR   (MEM_FLASH_PHY_BASE + MEM_FLAG_OFS)
#define BOOT_FLAG_NUM    (MEM_FLAG_SIZE / sizeof(ota_flag_t))

static const ota_flag_t *flag_latest(void)
{
    const ota_flag_t *rec;
    const ota_flag_t *best;
    uint32_t i;

    best = 0;
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
        if ((best == 0) || (rec[i].seq > best->seq))
        {
            best = &rec[i];
        }
    }
    return best;
}

static uint32_t slot_addr(uint8_t slot)
{
    uint32_t ofs;

    ofs = (slot == OTA_SLOT_A) ? MEM_SLOT_A_OFS : MEM_SLOT_B_OFS;
    return MEM_FLASH_RUN_BASE + ofs;
}

/* need_good 为 1 时，这一面的状态还必须是 GOOD。 */
static uint8_t face_ok(uint8_t slot, const ota_slot_info_t *info, uint8_t need_good)
{
    uint32_t addr;
    volatile img_hdr_t *hdr;

    if ((need_good != 0u) && (info->state != OTA_ST_GOOD))
    {
        return 0u;
    }
    if (info->size > MEM_SLOT_SIZE)
    {
        return 0u;
    }
    addr = slot_addr(slot);
    if (*(volatile uint32_t *)addr == MEM_FLASH_ERASED_WORD)
    {
        return 0u;
    }
    hdr = (volatile img_hdr_t *)(addr + MEM_IMG_HDR_OFS);
    if ((hdr->magic != MEM_IMG_MAGIC) || (hdr->link_ofs != (addr - MEM_FLASH_RUN_BASE)))
    {
        return 0u;
    }
    return (uint8_t)(boot_crc32((const uint8_t *)addr, info->size) == info->crc32);
}

uint32_t boot_choose(uint32_t *seq)
{
    const ota_flag_t *rec;
    uint8_t slot;
    uint8_t other;

    rec = flag_latest();
    if (rec == 0)
    {
        *seq = 0u;
        return slot_addr(OTA_SLOT_A);
    }
    *seq = rec->seq;
    if ((rec->run_slot != OTA_SLOT_A) && (rec->run_slot != OTA_SLOT_B))
    {
        return 0u;
    }
    slot = rec->run_slot;
    if (face_ok(slot, &rec->slot[slot], 0u) != 0u)
    {
        return slot_addr(slot);
    }
    other = (uint8_t)(slot ^ 1u);
    if (face_ok(other, &rec->slot[other], 1u) != 0u)
    {
        return slot_addr(other);
    }
    return 0u;
}
