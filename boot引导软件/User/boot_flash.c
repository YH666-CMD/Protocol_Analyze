#include "boot_flash.h"
#include "boot_crc.h"
#include "mem_map.h"
#include "ota_def.h"
#include "ch32v20x.h"
#include <stddef.h>

#define BOOT_FLAG_ADDR    (MEM_FLASH_PHY_BASE + MEM_FLAG_OFS)
#define BOOT_FLAG_NUM     (MEM_FLAG_SIZE / sizeof(ota_flag_t))
#define BOOT_FLAG_PAGE    (MEM_FLASH_PAGE / sizeof(ota_flag_t))

#define FLASH_KEY1        0x45670123u
#define FLASH_KEY2        0xCDEF89ABu
#define FLASH_PG          0x00000001u
#define FLASH_STRT        0x00000040u
#define FLASH_PAGE_ER     0x00020000u
#define FLASH_EHMOD       0x01000000u
#define FLASH_BSY         0x00000001u
#define FLASH_WRPRTERR    0x00000010u
#define FLASH_EOP         0x00000020u
#define FLASH_WAIT_N      200000u

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

static uint32_t irq_off(void)
{
    uint32_t state;

    __asm__ volatile ("csrr %0, 0x800" : "=r"(state));
    __asm__ volatile ("csrc 0x800, %0" :: "r"(0x88) : "memory");
    return state;
}

static void irq_on(uint32_t state)
{
    __asm__ volatile ("csrs 0x800, %0" :: "r"(state) : "memory");
}

/* BSY 卡住时取指也会停。这个计数只在还能取指时有用。 */
static uint8_t flash_wait(void)
{
    uint32_t n;

    n = FLASH_WAIT_N;
    while (((FLASH->STATR & FLASH_BSY) != 0u) && (n != 0u))
    {
        n--;
    }
    if (n == 0u)
    {
        return 0u;
    }
    if ((FLASH->STATR & FLASH_WRPRTERR) != 0u)
    {
        FLASH->STATR = FLASH_WRPRTERR;
        return 0u;
    }
    FLASH->STATR = FLASH_EOP;
    return 1u;
}

static void flash_unlock(void)
{
    if ((FLASH->CTLR & FLASH_CTLR_LOCK) != 0u)
    {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    if ((FLASH->CTLR & FLASH_CTLR_FAST_LOCK) != 0u)
    {
        FLASH->MODEKEYR = FLASH_KEY1;
        FLASH->MODEKEYR = FLASH_KEY2;
    }
}

static void flash_lock(void)
{
    FLASH->CTLR |= FLASH_CTLR_LOCK | FLASH_CTLR_FAST_LOCK;
}

static uint8_t is_blank(uint32_t addr, uint32_t len)
{
    uint32_t i;

    for (i = 0u; i < len; i += 4u)
    {
        if (*(volatile uint32_t *)(addr + i) != MEM_FLASH_ERASED_WORD)
        {
            return 0u;
        }
    }
    return 1u;
}

static uint8_t page_erase(uint32_t addr)
{
    uint32_t i;

    FLASH->CTLR |= FLASH_PAGE_ER;
    FLASH->ADDR = addr;
    FLASH->CTLR |= FLASH_STRT;
    if (flash_wait() == 0u)
    {
        FLASH->CTLR &= ~FLASH_PAGE_ER;
        return 0u;
    }
    FLASH->CTLR &= ~FLASH_PAGE_ER;
    for (i = 0u; i < MEM_FLASH_PAGE; i += 4u)
    {
        if (*(volatile uint32_t *)(addr + i) != MEM_FLASH_ERASED_WORD)
        {
            return 0u;
        }
    }
    return 1u;
}

/* 从低地址写到高地址。crc 在记录最后，所以它最后落地。 */
static uint8_t half_write(uint32_t addr, const ota_flag_t *rec)
{
    const uint8_t *src;
    uint32_t i;

    src = (const uint8_t *)rec;
    for (i = 0u; i < sizeof(ota_flag_t); i += 2u)
    {
        FLASH->CTLR |= FLASH_PG;
        *(volatile uint16_t *)(addr + i) = (uint16_t)(src[i] | ((uint16_t)src[i + 1u] << 8));
        if (flash_wait() == 0u)
        {
            FLASH->CTLR &= ~FLASH_PG;
            return 0u;
        }
        FLASH->CTLR &= ~FLASH_PG;
    }
    src = (const uint8_t *)rec;
    for (i = 0u; i < sizeof(ota_flag_t); i += 4u)
    {
        if (*(volatile uint32_t *)(addr + i) != *(const uint32_t *)(src + i))
        {
            return 0u;
        }
    }
    return 1u;
}

/* mode：0 把 run_slot 改成 slot 并清试运行；1 把 trial_cnt 记成 1；2 把 slot 标 BAD 并清试运行。 */
static uint8_t flag_save(const ota_flag_t *old, uint8_t mode, uint8_t slot)
{
    const ota_flag_t *base;
    const ota_flag_t *latest;
    ota_flag_t rec;
    uint32_t irq;
    uint32_t latest_i;
    uint32_t index;
    uint32_t n;
    uint32_t addr;
    uint32_t k;
    uint8_t ok;

    base = (const ota_flag_t *)BOOT_FLAG_ADDR;
    latest = flag_latest();
    latest_i = BOOT_FLAG_NUM;
    index = 0u;
    if (latest != 0)
    {
        latest_i = (uint32_t)(latest - base);
        index = latest_i + 1u;
        if (index >= BOOT_FLAG_NUM)
        {
            index = 0u;
        }
    }

    for (k = 0u; k < (sizeof(ota_flag_t) / 4u); k++)
    {
        ((uint32_t *)&rec)[k] = ((const uint32_t *)old)[k];
    }
    if (mode == 0u)
    {
        rec.run_slot = slot;
        rec.trial_slot = OTA_SLOT_NONE;
        rec.trial_cnt = 0u;
    }
    else if (mode == 1u)
    {
        rec.trial_cnt = 1u;
    }
    else
    {
        rec.slot[slot].state = OTA_ST_BAD;
        rec.trial_slot = OTA_SLOT_NONE;
        rec.trial_cnt = 0u;
    }
    rec.magic = OTA_FLAG_MAGIC;
    rec.seq = (latest == 0) ? 1u : (latest->seq + 1u);
    rec.crc = boot_crc32((const uint8_t *)&rec, (uint32_t)offsetof(ota_flag_t, crc));

    if ((FLASH->CTLR & FLASH_EHMOD) != 0u)
    {
        return 0u;
    }
    irq = irq_off();
    flash_unlock();
    ok = 0u;
    for (n = 0u; n < BOOT_FLAG_NUM; n++)
    {
        if (index == latest_i)
        {
            break;
        }
        addr = BOOT_FLAG_ADDR + (index * sizeof(ota_flag_t));
        if ((index % BOOT_FLAG_PAGE) == 0u)
        {
            if ((latest_i < BOOT_FLAG_NUM) && ((latest_i / BOOT_FLAG_PAGE) == (index / BOOT_FLAG_PAGE)))
            {
                break;
            }
            if (page_erase(addr) == 0u)
            {
                break;
            }
        }
        if (is_blank(addr, sizeof(ota_flag_t)) != 0u)
        {
            ok = half_write(addr, &rec);
            break;
        }
        index++;
        if (index >= BOOT_FLAG_NUM)
        {
            index = 0u;
        }
    }
    flash_lock();
    irq_on(irq);
    return ok;
}

uint32_t boot_choose(uint32_t *seq)
{
    const ota_flag_t *rec;
    uint8_t slot;
    uint8_t other;
    uint8_t trial;
    uint8_t banned;
    uint8_t state;

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

    banned = OTA_SLOT_NONE;
    trial = rec->trial_slot;
    if ((trial == OTA_SLOT_A) || (trial == OTA_SLOT_B))
    {
        state = rec->slot[trial].state;
        if ((rec->trial_cnt < OTA_TRIAL_MAX) &&
            ((state == OTA_ST_READY) || (state == OTA_ST_GOOD)) &&
            (face_ok(trial, &rec->slot[trial], 0u) != 0u))
        {
            if (flag_save(rec, 1u, 0u) != 0u)
            {
                *seq = *seq + 1u;
                return slot_addr(trial);
            }
        }
        else
        {
            if (flag_save(rec, 2u, trial) != 0u)
            {
                *seq = *seq + 1u;
            }
            banned = trial;
        }
    }

    slot = rec->run_slot;
    if ((slot != banned) && (face_ok(slot, &rec->slot[slot], 0u) != 0u))
    {
        return slot_addr(slot);
    }
    other = (uint8_t)(slot ^ 1u);
    if ((other != banned) && (face_ok(other, &rec->slot[other], 1u) != 0u))
    {
        if (flag_save(rec, 0u, other) != 0u)
        {
            *seq = *seq + 1u;
        }
        return slot_addr(other);
    }
    return 0u;
}
