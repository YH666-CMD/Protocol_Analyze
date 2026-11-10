#include "bsp_time.h"
#include "bsp_clk.h"
#include "config.h"
#include "ch32v20x.h"
#include <stddef.h>

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
static bsp_time_ms_fn s_ms_hook;

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/* 清掉中断标志，毫秒加一，再做上层挂上的检查。没挂钩子就只计时。 */
void SysTick_Handler(void)
{
    uint32_t now;

    SysTick->SR = 0;
    s_ms++;
    now = s_ms;
    if (s_ms_hook != NULL)
    {
        s_ms_hook(now);
    }
}

/* 主频不够分出 1 毫秒，时基不能启动。 */
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
    s_ms_hook = NULL;
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

/* 空闲时马上返回。协议分析仪不睡眠。
 * USB 写分组缓冲时 CPU 必须醒着，进去睡觉会丢掉 SETUP。 */
static void time_wait(void)
{
}

/* 上电配置时先清空。调度器准备好以后再挂上。 */
static void time_set_ms_hook(bsp_time_ms_fn hook)
{
    s_ms_hook = hook;
}

static uint32_t time_cycles(void)
{
    return (uint32_t)SysTick->CNT;
}

/* 调度器在测时打开时调用。现在没有打印口，只把参数留在接口上 */
static result_t time_print_task(const char *name, uint32_t cost_us)
{
    (void)name;
    (void)cost_us;
    return RES_OK;
}

/* 毫秒和本毫秒的计数要属于同一时刻。读的过程中毫秒变了就重读。 */
static uint32_t time_now_us(void)
{
    uint32_t ms;
    uint32_t cnt;
    uint32_t per_us;

    per_us = bsp_clk.pf_core_hz() / 1000000u;
    if (per_us == 0u)
    {
        return 0u;
    }

    do
    {
        ms = s_ms;
        cnt = (uint32_t)SysTick->CNT;
    } while (ms != s_ms);

    return (ms * 1000u) + (cnt / per_us);
}

const bsp_time_driver_t bsp_time =
{
    .pf_init    = time_init,
    .pf_start   = time_start,
    .pf_now_ms  = time_now_ms,
    .pf_cycles  = time_cycles,
    .pf_now_us       = time_now_us,
    .pf_print_task   = time_print_task,
    .pf_wait         = time_wait,
    .pf_set_ms_hook  = time_set_ms_hook,
};
