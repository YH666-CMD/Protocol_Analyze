#include "bsp_board.h"
#include "bsp_clk.h"
#include "bsp_gpio.h"
#include "bsp_time.h"
#include "platform.h"
#include "ch32v20x.h"
#include "ch32v20x_rcc.h"

#define BOARD_UID_BASE  ((uint32_t)0x1FFFF7E8)

/* 上电后写入。下标 0 为低 32 位，1 为中 32 位，2 为高 32 位。留给调试器看。 */
uint32_t g_mcu_uid[3];

static void board_read_uid(void)
{
    g_mcu_uid[0] = *(volatile uint32_t *)(BOARD_UID_BASE);
    g_mcu_uid[1] = *(volatile uint32_t *)(BOARD_UID_BASE + 4u);
    g_mcu_uid[2] = *(volatile uint32_t *)(BOARD_UID_BASE + 8u);
}

/* 同时置位时只留一个：看门狗优先于软件复位，上电优先于引脚。读完就清掉，避免下次上电还看见旧标志。 */
static plat_reset_t board_reset_cause(void)
{
    plat_reset_t reason;

    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
        reason = PLAT_RESET_IWDG;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
    {
        reason = PLAT_RESET_WWDG;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
    {
        reason = PLAT_RESET_SOFTWARE;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) != RESET)
    {
        reason = PLAT_RESET_LOWPOWER;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
        reason = PLAT_RESET_POR;
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
        reason = PLAT_RESET_PIN;
    }
    else
    {
        reason = PLAT_RESET_UNKNOWN;
    }
    RCC_ClearFlag();
    return reason;
}

/* 先读复位原因并回调，再开时钟和引脚。时基只配置，不在这里开始计数。 */
void bsp_board_init(void)
{
    (void)plat_on_reset((uint32_t)board_reset_cause());
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);/*0-中断间不能抢占，1-高优先级中断能抢占低优先级 bit6:5子优先级bit4:0保留*/
    board_read_uid();
    bsp_clk.pf_init();
    bsp_gpio.pf_init();
    bsp_time.pf_init();
}
/*开总中断*/
void bsp_board_irq_on(void)
{
    __enable_irq();
}
/*关总中断，返回关之前的状态*/
uint32_t bsp_board_irq_lock(void)
{
    uint32_t state;

    __asm__ volatile("csrr %0, 0x800" : "=r"(state));
    __asm__ volatile("csrc 0x800, %0" :: "r"(0x88) : "memory");
    return state;
}
/*不是开中断是恢复到lock之前的状态*/
void bsp_board_irq_unlock(uint32_t state)
{
    __asm__ volatile("csrw 0x800, %0" :: "r"(state) : "memory");
}
