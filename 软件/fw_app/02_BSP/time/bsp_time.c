#include "bsp_time.h"
#include "bsp_clk.h"
#include "config.h"
#include "ch32v20x.h"

/*
 * 沁恒 SysTick CTLR：
 *   bit0 STE  计数使能
 *   bit1 STIE 中断使能
 *   bit2 STCLK 1 为 HCLK，0 为 HCLK/8
 *   bit3 STRE 自动重装
 * Delay_Us / Delay_Ms 会改同一只 SysTick 的 CMP 和 CTLR。时基启动后不要再调用它们。
 * User/ch32v20x_it.c 没有 SysTick_Handler，启动文件里是 weak，强符号放在本文件。
 */
#define SYSTICK_CTLR_STE    (1u << 0)
#define SYSTICK_CTLR_STIE   (1u << 1)
#define SYSTICK_CTLR_STCLK  (1u << 2)
#define SYSTICK_CTLR_STRE   (1u << 3)
#define SYSTICK_CTLR_INIT   (1u << 5)

static volatile uint32_t s_ms;

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void SysTick_Handler(void)
{
    SysTick->SR = 0;
    s_ms++;
}

static void time_halt(void)
{
    for (;;)
    {
        __asm__ volatile("" ::: "memory");
    }
}

static void time_init(void)
{
    uint32_t hz;
    uint32_t ticks;

    s_ms = 0;
    hz = bsp_clk.pf_core_hz();
    if (hz < SCHED_TICK_HZ)
    {
        time_halt();
    }

    ticks = hz / SCHED_TICK_HZ;

    SysTick->CTLR = 0;
    SysTick->SR = 0;
    SysTick->CNT = 0;
    /* 向上计数，到 CMP 产生中断后回到 0。周期是 CMP + 1 个 HCLK。 */
    SysTick->CMP = (uint64_t)ticks - 1u;
    SysTick->CTLR = SYSTICK_CTLR_STCLK | SYSTICK_CTLR_STRE;
}

static void time_start(void)
{
    /* STIE 只让定时器产生请求。PFIC 里这一位不开，请求到不了 SysTick_Handler。 */
    NVIC_SetPriority(SysTick_IRQn, 0x00);
    NVIC_EnableIRQ(SysTick_IRQn);

    SysTick->CNT = 0;
    SysTick->SR = 0;
    SysTick->CTLR |= SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_INIT;
}

static uint32_t time_now_ms(void)
{
    return s_ms;
}

const bsp_time_driver_t bsp_time =
{
    .pf_init   = time_init,
    .pf_start  = time_start,
    .pf_now_ms = time_now_ms,
};
