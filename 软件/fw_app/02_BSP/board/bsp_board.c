#include "bsp_board.h"
#include "bsp_clk.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "ch32v20x.h"

#define BOARD_UID_BASE  ((uint32_t)0x1FFFF7E8)

/* 上电后写入。下标 0 为低 32 位，1 为中 32 位，2 为高 32 位。留给调试器看。 */
uint32_t g_mcu_uid[3];

static void board_read_uid(void)
{
    g_mcu_uid[0] = *(volatile uint32_t *)(BOARD_UID_BASE);
    g_mcu_uid[1] = *(volatile uint32_t *)(BOARD_UID_BASE + 4u);
    g_mcu_uid[2] = *(volatile uint32_t *)(BOARD_UID_BASE + 8u);
}

void bsp_board_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);/*0-中断间不能抢占，1-高优先级中断能抢占低优先级 bit6:5子优先级bit4:0保留*/
    board_read_uid();
    bsp_clk.pf_init();
    bsp_gpio.pf_init();
    bsp_time.pf_init();
}

void bsp_board_irq_on(void)
{
    __enable_irq();
}
