#include "bsp_flash.h"
#include "config.h"
#include "bsp_board.h"
#include "ch32v20x.h"

#define FLASH_KEY1       0x45670123u
#define FLASH_KEY2       0xCDEF89ABu
#define FLASH_LOCK       0x00000080u
#define FLASH_FLOCK      0x00008000u
#define FLASH_PG         0x00000001u
#define FLASH_STRT       0x00000040u
#define FLASH_PAGE_PG    0x00010000u
#define FLASH_PAGE_ER    0x00020000u
#define FLASH_PG_STRT    0x00200000u
#define FLASH_EHMOD      0x01000000u
#define FLASH_BSY        0x00000001u
#define FLASH_WR_BSY     0x00000002u
#define FLASH_WRPRTERR   0x00000010u
#define FLASH_EOP        0x00000020u
#define FLASH_WAIT_N     200000u

/* BSY 卡住时取指也会停，这个计数只在还能取指时有用。 */
static result_t flash_wait(uint32_t mask)
{
    uint32_t n;

    n = FLASH_WAIT_N;
    while (((FLASH->STATR & mask) != 0u) && (n != 0u))
    {
        n--;
    }
    if (n == 0u)
    {
        return RES_STATE;
    }
    if ((FLASH->STATR & FLASH_WRPRTERR) != 0u)
    {
        FLASH->STATR = FLASH_WRPRTERR;
        return RES_STATE;
    }
    FLASH->STATR = FLASH_EOP;
    return RES_OK;
}

static void flash_unlock(void)
{
    if ((FLASH->CTLR & FLASH_LOCK) != 0u)
    {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    if ((FLASH->CTLR & FLASH_FLOCK) != 0u)
    {
        FLASH->MODEKEYR = FLASH_KEY1;
        FLASH->MODEKEYR = FLASH_KEY2;
    }
}

static void flash_lock(void)
{
    FLASH->CTLR |= FLASH_LOCK | FLASH_FLOCK;
}

static uint32_t flash_phy(uint32_t addr)
{
    if (addr < MEM_FLASH_PHY_BASE)
    {
        addr += MEM_FLASH_PHY_BASE;
    }
    return addr;
}

static uint8_t flash_inside(uint32_t addr, uint32_t len, uint32_t base, uint32_t size)
{
    if ((len == 0u) || (len > size) || (addr < base))
    {
        return 0u;
    }
    return (uint8_t)((addr - base) <= (size - len));
}

static uint8_t flash_hit(uint32_t addr, uint32_t len, uint32_t base, uint32_t size)
{
    if ((len == 0u) || (size == 0u))
    {
        return 0u;
    }
    return (uint8_t)((addr < (base + size)) && (base < (addr + len)));
}

/* 1：这一段可以写。碰到 Boot 或当前面时先记一笔。 */
static result_t flash_allow(uint32_t addr, uint32_t len)
{
    uint32_t cur;
    uint32_t other;

    cur = MEM_FLASH_PHY_BASE + ((APP_SLOT == 0) ? MEM_SLOT_A_OFS : MEM_SLOT_B_OFS);
    other = MEM_FLASH_PHY_BASE + ((APP_SLOT == 0) ? MEM_SLOT_B_OFS : MEM_SLOT_A_OFS);
    if (flash_hit(addr, len, MEM_FLASH_PHY_BASE + MEM_BOOT_OFS, MEM_BOOT_AREA_SIZE) ||
        flash_hit(addr, len, cur, MEM_SLOT_SIZE))
    {
        (void)plat_raise(&plat_hook_flash, addr, len);
        return RES_PARAM;
    }
    if (flash_inside(addr, len, other, MEM_SLOT_SIZE) ||
        flash_inside(addr, len, MEM_FLASH_PHY_BASE + MEM_FLAG_OFS, MEM_FLAG_SIZE) ||
        flash_inside(addr, len, MEM_FLASH_PHY_BASE + MEM_PARAM_OFS, MEM_PARAM_SIZE) ||
        flash_inside(addr, len, MEM_FLASH_PHY_BASE + MEM_CRASH_OFS, MEM_CRASH_SIZE) ||
        flash_inside(addr, len, MEM_FLASH_PHY_BASE + MEM_LOG_OFS, MEM_LOG_SIZE))
    {
        return RES_OK;
    }
    return RES_PARAM;
}

static result_t flash_guard(uint32_t *addr, uint32_t len, uint32_t align)
{
    if (((*addr & (align - 1u)) != 0u) || ((len & (align - 1u)) != 0u) || (len == 0u))
    {
        return RES_PARAM;
    }
    *addr = flash_phy(*addr);
    return flash_allow(*addr, len);
}

static uint8_t flash_erased_byte(uint32_t addr)
{
    return ((addr & 2u) == 0u) ? 0x39u : 0xE3u;
}

static uint8_t flash_is_blank(uint32_t addr, uint32_t len)
{
    uint32_t i;

    if (len == 0u)
    {
        return 0u;
    }
    addr = flash_phy(addr);
    if ((addr < MEM_FLASH_PHY_BASE) ||
        ((addr + len) > (MEM_FLASH_PHY_BASE + MEM_FLASH_TOTAL)))
    {
        return 0u;
    }
    for (i = 0u; i < len; i++)
    {
        if (*(volatile uint8_t *)(addr + i) != flash_erased_byte(addr + i))
        {
            return 0u;
        }
    }
    return 1u;
}

static result_t flash_erase_do(uint32_t addr, uint32_t len)
{
    uint32_t page;
    uint32_t i;
    result_t rc;

    for (page = addr; page < (addr + len); page += MEM_FLASH_PAGE)
    {
        FLASH->CTLR |= FLASH_PAGE_ER;
        FLASH->ADDR = page;
        FLASH->CTLR |= FLASH_STRT;
        rc = flash_wait(FLASH_BSY);
        FLASH->CTLR &= ~FLASH_PAGE_ER;
        if (rc != RES_OK)
        {
            return rc;
        }
        for (i = 0u; i < MEM_FLASH_PAGE; i += 4u)
        {
            if (*(volatile uint32_t *)(page + i) != MEM_FLASH_ERASED_WORD)
            {
                return RES_STATE;
            }
        }
    }
    return RES_OK;
}

static result_t flash_write_do(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t i;
    result_t rc;

    for (i = 0u; i < len; i += 2u)
    {
        FLASH->CTLR |= FLASH_PG;
        *(volatile uint16_t *)(addr + i) = (uint16_t)(buf[i] | ((uint16_t)buf[i + 1u] << 8));
        rc = flash_wait(FLASH_BSY);
        FLASH->CTLR &= ~FLASH_PG;
        if (rc != RES_OK)
        {
            return rc;
        }
    }
    for (i = 0u; i < len; i++)
    {
        if (*(volatile uint8_t *)(addr + i) != buf[i])
        {
            return RES_STATE;
        }
    }
    return RES_OK;
}

static result_t flash_page_do(uint32_t addr, const uint32_t *buf)
{
    uint32_t i;
    uint32_t at;
    result_t rc;

    FLASH->CTLR |= FLASH_PAGE_PG;
    rc = flash_wait(FLASH_BSY | FLASH_WR_BSY);
    if (rc != RES_OK)
    {
        FLASH->CTLR &= ~FLASH_PAGE_PG;
        return rc;
    }
    at = addr;
    for (i = 0u; i < (MEM_FLASH_PAGE / 4u); i++)
    {
        *(volatile uint32_t *)at = buf[i];
        at += 4u;
        rc = flash_wait(FLASH_WR_BSY);
        if (rc != RES_OK)
        {
            FLASH->CTLR &= ~FLASH_PAGE_PG;
            return rc;
        }
    }
    FLASH->CTLR |= FLASH_PG_STRT;
    rc = flash_wait(FLASH_BSY);
    FLASH->CTLR &= ~FLASH_PAGE_PG;
    if (rc != RES_OK)
    {
        return rc;
    }
    for (i = 0u; i < (MEM_FLASH_PAGE / 4u); i++)
    {
        if (*(volatile uint32_t *)(addr + (i * 4u)) != buf[i])
        {
            return RES_STATE;
        }
    }
    return RES_OK;
}

static result_t flash_session(result_t (*op)(uint32_t, const void *, uint32_t),
                              uint32_t addr, const void *buf, uint32_t len)
{
    uint32_t irq;
    result_t rc;

    if ((FLASH->CTLR & FLASH_EHMOD) != 0u)
    {
        return RES_STATE;
    }
    irq = bsp_board_irq_lock();
    flash_unlock();
    rc = op(addr, buf, len);
    flash_lock();
    bsp_board_irq_unlock(irq);
    return rc;
}

static result_t erase_op(uint32_t addr, const void *buf, uint32_t len)
{
    (void)buf;
    return flash_erase_do(addr, len);
}

static result_t write_op(uint32_t addr, const void *buf, uint32_t len)
{
    return flash_write_do(addr, (const uint8_t *)buf, len);
}

static result_t page_op(uint32_t addr, const void *buf, uint32_t len)
{
    (void)len;
    return flash_page_do(addr, (const uint32_t *)buf);
}

static result_t flash_erase(uint32_t addr, uint32_t len)
{
    result_t rc;

    rc = flash_guard(&addr, len, MEM_FLASH_PAGE);
    if (rc != RES_OK)
    {
        return rc;
    }
    return flash_session(erase_op, addr, 0, len);
}

static result_t flash_write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    result_t rc;

    if (buf == 0)
    {
        return RES_PARAM;
    }
    rc = flash_guard(&addr, len, 2u);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (flash_is_blank(addr, len) == 0u)
    {
        return RES_STATE;
    }
    return flash_session(write_op, addr, buf, len);
}

static result_t flash_write_page(uint32_t addr, const uint32_t *buf)
{
    result_t rc;

    if (buf == 0)
    {
        return RES_PARAM;
    }
    rc = flash_guard(&addr, MEM_FLASH_PAGE, MEM_FLASH_PAGE);
    if (rc != RES_OK)
    {
        return rc;
    }
    if (flash_is_blank(addr, MEM_FLASH_PAGE) == 0u)
    {
        return RES_STATE;
    }
    return flash_session(page_op, addr, buf, MEM_FLASH_PAGE);
}

const bsp_flash_driver_t bsp_flash =
{
    flash_erase,
    flash_write,
    flash_write_page,
    flash_is_blank
};
