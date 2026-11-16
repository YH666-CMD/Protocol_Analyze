#include "bsp_wdt.h"
#include "config.h"

#if (WDT_HW_TIMEOUT_MS <= WDT_ROUND_MAX_MS)
#error "WDT_HW_TIMEOUT_MS must be greater than WDT_ROUND_MAX_MS"
#endif

#include "ch32v20x_iwdg.h"

#if WDT_ENABLE

/* 独立看门狗用内部低速时钟，40kHz。分频 256 时 4000 ms 对应重装值 624。 */
#define WDT_LSI_HZ        40000u
#define WDT_PRESCALER_DIV 256u
#define WDT_TICKS         ((WDT_HW_TIMEOUT_MS * (WDT_LSI_HZ / 1000u)) / WDT_PRESCALER_DIV)
#define WDT_RELOAD        (WDT_TICKS - 1u)

#if (WDT_TICKS == 0u) || (WDT_TICKS > 4096u)
#error "WDT_HW_TIMEOUT_MS does not fit the watchdog reload register"
#endif
#endif

/* 给紧急日志留出写 RAM 的时间。不用 Delay_Ms，避免它改掉时基。 */
static void wdt_wait_for_record(void)
{
    volatile uint32_t spin;

    for (spin = 0u; spin < WDT_LOG_WAIT_SPIN; spin++)
    {
        __asm__ volatile("" ::: "memory");
    }
}

/* 重启前的专用钩子。打印或另存现场以后写在这里，现在什么都不做。 */
static result_t wdt_before_reset(uint32_t round_ms)
{
    (void)round_ms;
    return RES_OK;
}

/* 总开关为 0 时这里什么都不写。独立看门狗一旦打开就不能关。 */
static void wdt_start(void)
{
#if WDT_ENABLE
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload((uint16_t)WDT_RELOAD);
    IWDG_ReloadCounter();
    IWDG_Enable();
#endif
}

/* 把独立看门狗调到最短超时。分频 4、重装 0，大约 0.1 ms 后复位。 */
static void wdt_arm_now(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_4);
    IWDG_SetReload(0u);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/* 手动重启。钩子返回后空等一小段，再让看门狗复位。正常到不了返回。 */
static result_t wdt_reset(void)
{
    volatile uint32_t spin;

    (void)wdt_before_reset(0u);
    wdt_wait_for_record();
    wdt_arm_now();
    for (spin = 0u; spin < 0xFFFFFFFFu; spin++)
    {
        __asm__ volatile("" ::: "memory");
    }
    return RES_STATE;
}

/* 超时就不喂。硬件超时后芯片复位。总开关为 0 时同样走钩子，但不写寄存器 */
static result_t wdt_kick(uint32_t round_ms)
{
    if (round_ms > WDT_ROUND_MAX_MS)
    {
        (void)plat_raise(&plat_hook_wdt_hang, round_ms, 0u);
        (void)wdt_before_reset(round_ms);
        wdt_wait_for_record();
        return RES_OVERRUN;
    }
#if WDT_ENABLE
    IWDG_ReloadCounter();
#else
    (void)round_ms;
#endif
    return RES_OK;
}

const bsp_wdt_driver_t bsp_wdt =
{
    .pf_start        = wdt_start,
    .pf_kick         = wdt_kick,
    .pf_before_reset = wdt_before_reset,
    .pf_reset        = wdt_reset,
};
