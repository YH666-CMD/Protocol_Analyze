/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: Boot。
 *       看 A 面。地址、擦除值和镜像头在软件工程的 mem_map.h。
 *       起始字是擦除值，或镜像头不是这一面，就停住。
 *       否则在软件中断里跳过去。选 A 还是 B、读标志区，后面再做。
 *
 *       g_boot_trace
 *         1  已进入 main，准备看目标面
 *         2  起始字是擦除值，停在这里
 *         3  头核对通过，准备跳到 g_boot_dest
 *         4  魔术字不对，或 link_ofs 不是这一面，停在这里
 *       g_boot_dest  准备跳的地址。停在 2 或 4 的时候也是这个地址。
 *******************************************************************************/
#include "ch32v20x.h"
#include "mem_map.h"
#include "ota_def.h"

#define BOOT_A_ADDR       (MEM_FLASH_RUN_BASE + MEM_SLOT_A_OFS)

volatile uint32_t g_boot_trace;
volatile uint32_t g_boot_dest;

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
    boot_jump(BOOT_A_ADDR);
}
