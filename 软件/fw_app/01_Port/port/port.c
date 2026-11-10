#include "port.h"
#include "platform.h"
#include "config.h"
#include "bsp_board.h"
#include "bsp_clk.h"
#include "bsp_time.h"
#include "bsp_wdt.h"
#include "sched.h"
#include "sched_tasks.h"
#include "evt.h"
#include "log.h"
#include <stddef.h>

/* 上电自检记下的时钟源。不在这里改时钟。 */
bsp_clk_src_t g_clk_source;

static sched_t s_sched;

/* 时基启动后毫秒必须变化。转满仍不变，就当定时器没有走。 */
static void port_wait_timebase(void)
{
    uint32_t start;
    volatile uint32_t spin;

    start = bsp_time.pf_now_ms();
    for (spin = 0u; spin < PORT_TIMEBASE_SPIN; spin++)
    {
        if (bsp_time.pf_now_ms() != start)
        {
            return;
        }
    }
    (void)plat_raise(&plat_hook_timebase_dead, spin, start);
}

/* 上电顺序：板级、紧急日志、事件、调度。时基走起来之后再开看门狗。 */
void app_init(void)
{
    const sched_task_t *table;
    sched_task_rt_t *rt;
    uint8_t count;
    sched_now_us_fn now_us;
    sched_print_fn print_task;

    table = NULL;
    rt = NULL;
    count = 0u;

    bsp_board_init();
    /* 时钟起来之后第一件事：紧急 RAM 里如果有标志，就尝试写入 FLASH。 */
    (void)log_sw_driver.pf_boot();
    (void)evt_sw_driver.pf_init(SCHED_ID_COUNT, bsp_board_irq_lock, bsp_board_irq_unlock);
    sched_tasks_bind(&table, &rt, &count);

#if SCHED_TIME_MEASURE_ENABLE
    now_us = bsp_time.pf_now_us;
    print_task = bsp_time.pf_print_task;
#else
    now_us = NULL;
    print_task = NULL;
#endif
    (void)sched_sw_driver.pf_init(&s_sched, table, rt, count, bsp_time.pf_now_ms,
                                  now_us, print_task, bsp_wdt.pf_kick,
                                  bsp_time.pf_wait, plat_on_task_stuck);
    /* 时基开始走之前挂上。每一毫秒检查有没有任务一直不返回。 */
    bsp_time.pf_set_ms_hook(sched_sw_driver.pf_on_ms);
    bsp_time.pf_start();
    bsp_board_irq_on();
    port_wait_timebase();
    g_clk_source = bsp_clk.pf_source();
    bsp_wdt.pf_start();
}

/* 主循环一次只推进一轮调度。 */
void app_loop(void)
{
    (void)sched_sw_driver.pf_poll(&s_sched);
}
