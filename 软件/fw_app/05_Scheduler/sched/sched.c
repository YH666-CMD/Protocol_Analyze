#include "sched.h"
#include <stddef.h>

/* 丢拍阈值留在调度器内部，不进 config.h，也不对外提供读取接口。 */
#define SCHED_SLIP_WARN_MS  5u

static const sched_task_t *s_table;/*任务表指针*/
static sched_task_rt_t    *s_rt;/*运行时数组指针*/
static uint8_t             s_count;/*表里有几个任务*/
static sched_now_ms_fn     s_now_ms;/*读毫秒的函数指针*/
static sched_hook_fn       s_idle;/*空闲钩子：毫秒没变时调用*/
static sched_hook_fn       s_kick;/*收尾钩子：任务都跑完后调用*/
static uint32_t            s_last_ms;/*上一次处理过的毫秒值*/
static volatile uint32_t   s_slip;/*丢拍次数*/
static bool                s_ready;/*初始化成功置True*/

/*死循环+汇编，异常时保留现场pc指针指向这里调用栈完整保留*/
static void sched_halt(void)
{
    for (;;)
    {
        __asm__ volatile("" ::: "memory");
    }
}

/*用相位错开各个任务*/
static uint32_t sched_next_phase(uint32_t now, uint16_t period, uint16_t phase)
{
    uint32_t due;

    due = now - (now % period) + phase;
    if (due <= now)
    {
        due += period;
    }
    return due;
}
/*时基和相位关系是否合法*/
static void sched_check_task(const sched_task_t *task)
{
    if (task->period_ms == 0u)
    {
        if (task->phase_ms != 0u)
        {
            sched_halt();
        }
        return;
    }

    if (task->phase_ms >= task->period_ms || task->pf_on_tick == NULL)
    {
        sched_halt();
    }
}
/*初始化每个任务的基本信息*/
static void sched_init(const sched_task_t *table, sched_task_rt_t *rt, uint8_t count,
                       sched_now_ms_fn now_ms, sched_hook_fn idle, sched_hook_fn kick)
{
    uint32_t now;
    uint8_t i;

    if (now_ms == NULL)
    {
        sched_halt();
    }
    if (count > 0u && (table == NULL || rt == NULL))
    {
        sched_halt();
    }

    for (i = 0u; i < count; i++)
    {
        sched_check_task(&table[i]);
    }

    now = now_ms();
    for (i = 0u; i < count; i++)
    {
        rt[i].last_run = now;
        if (table[i].period_ms == 0u)
        {
            rt[i].next_due = 0u;
        }
        else
        {
            rt[i].next_due = sched_next_phase(now, table[i].period_ms, table[i].phase_ms);
        }

        if (table[i].pf_init != NULL)
        {
            table[i].pf_init();
        }
    }

    s_table = table;
    s_rt = rt;
    s_count = count;
    s_now_ms = now_ms;
    s_idle = idle;
    s_kick = kick;
    s_last_ms = now;
    s_slip = 0u;
    s_ready = true;
}
/*周期调用任务*/
static void sched_poll(void)
{
    uint32_t now;
    uint32_t elapsed;
    uint8_t i;

    if (!s_ready) return;

    now = s_now_ms();
    if (now == s_last_ms)
    {
        if (s_idle != NULL)
        {
            s_idle();
        }
        return;
    }

    elapsed = now - s_last_ms;
    s_last_ms = now;
    if (elapsed > SCHED_SLIP_WARN_MS && s_slip < UINT32_MAX)
    {
        s_slip++;
    }

    for (i = 0u; i < s_count; i++)
    {
        sched_task_rt_t *rt;
        uint32_t spent;

        if (s_table[i].period_ms == 0u) continue;

        rt = &s_rt[i];
        if ((uint32_t)(now - rt->next_due) >= 0x80000000u) continue;//负数还没到跳出去，正数到了往下走

        spent = now - rt->last_run;
        s_table[i].pf_on_tick(now, spent);
        rt->last_run = now;
        rt->next_due += s_table[i].period_ms;
        /*一般不会走到下个if这里，只有这个任务已经迟到了至少一个完整周期，作用是预防补跑的*/
        //同一拍里排在前面的任务耗时长，导致下一次轮询间隔变大1 ms 任务余量最小，最容易被拖到这一拍。
        //多个 1 ms 任务的总耗时超过一个节拍。
        //某个任务的执行时间接近或超过它自己的周期。
        //驱动里 while 等硬件标志（I2C/SPI/USB 等）
        //Flash 擦除/编程的等待循环，一次可能几十毫秒。
        //任务里调用 Delay_Ms / Delay_Us（Delay_Us 还会改 SysTick 的 CMP）。
        //中断太频繁，或者 idle 钩子耗时过长，主循环很久才轮询一次。
        //时基配置得比 1 ms 快，s_ms 走得比真实时间快。
        if ((uint32_t)(now - rt->next_due) < 0x80000000u)
        {
            rt->next_due = now + s_table[i].period_ms;
        }
    }
    /*只要有一个任务不执行就喂狗*/
    if (s_kick != NULL)
    {
        s_kick();
    }
}

const sched_driver_interface_t sched_sw_driver =
{
    .pf_init = sched_init,
    .pf_poll = sched_poll,
};
