/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: Boot。
 *       地址、擦除值和镜像头在软件工程的 mem_map.h。
 *       没有记录就跳 A 面，不算整面 CRC。
 *       有试运行面，且次数还没到上限、状态是 READY 或 GOOD、校验通过，
 *       就把次数写成 1 再跳过去。次数写不上就不跳这一面。
 *       次数已经用过，或这一面校验不过，就标 BAD 并清掉试运行，再按 run_slot 走。
 *       run_slot 不过，且另一面是 GOOD 并且校验通过，就跳另一面，
 *       并追加一条记录，run_slot 改成跳过去的那一面。
 *       两面都不行就停住。回退写失败仍然跳，下次复位再试。
 *       起始字是擦除值，或镜像头不是这一面，也停住。
 *       改记录后要复位，Boot 才会再选。
 *
 *       g_boot_trace
 *         1  已进入 main，准备看目标面
 *         2  起始字是擦除值，停在这里
 *         3  头核对通过，准备跳到 g_boot_dest
 *         4  魔术字不对，或 link_ofs 不是这一面，停在这里
 *         5  记录里的两面都不可用，停在这里
 *       g_boot_dest  准备跳的地址。停在 2、4、5 的时候也看它。5 的时候是 0。
 *       g_boot_seq   标志区里有效记录的最大序号。没有记录是 0。
 *******************************************************************************/
#include "ch32v20x.h"
#include "mem_map.h"
#include "ota_def.h"
#include "boot_flash.h"

volatile uint32_t g_boot_trace;
volatile uint32_t g_boot_dest;
volatile uint32_t g_boot_seq;

static volatile uint32_t s_jump_addr;

void SW_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void SW_Handler(void)
{
    uint32_t addr;

    addr = s_jump_addr;
    __asm volatile ("jr %0" : : "r"(addr));
    while (1)
    {
    }
}

static void boot_jump(uint32_t addr)
{
    volatile img_hdr_t *hdr;

    g_boot_trace = 1;
    g_boot_dest = addr;
    if (*(volatile uint32_t *)addr == MEM_FLASH_ERASED_WORD)
    {
        g_boot_trace = 2;
        while (1)
        {
        }
    }

    hdr = (volatile img_hdr_t *)(addr + MEM_IMG_HDR_OFS);
    if ((hdr->magic != MEM_IMG_MAGIC) ||
        (hdr->link_ofs != (addr - MEM_FLASH_RUN_BASE)))
    {
        g_boot_trace = 4;
        while (1)
        {
        }
    }

    s_jump_addr = addr;
    g_boot_trace = 3;
    FLASH->CTLR |= FLASH_CTLR_LOCK | FLASH_CTLR_FAST_LOCK;
    NVIC_EnableIRQ(Software_IRQn);
    NVIC_SetPendingIRQ(Software_IRQn);
    while (1)
    {
    }
}

int main(void)
{
    uint32_t seq;
    uint32_t addr;

    addr = boot_choose(&seq);
    g_boot_seq = seq;
    if (addr == 0u)
    {
        g_boot_trace = 5u;
        g_boot_dest = 0u;
        while (1)
        {
        }
    }
    boot_jump(addr);
}
