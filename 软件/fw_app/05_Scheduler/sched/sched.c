#include "sched.h"
#include "evt.h"
#include <stddef.h>
#include <stdbool.h>

#define SCHED_EVT_BURST          4u        /*一次 sched_events 最多连续处理多少个事件*/
#define SCHED_STUCK_FALLBACK_MS  1000u     /*没有周期（period_ms == 0）的任务，卡死判断时用的兜底上限*/
#define SCHED_RUN_NONE           0xFFu     /*run_id 的哨兵值：255 表示“现在不在任何任务函数里*/

static sched_t *s_bound;    /* s_bound：pf_init 保存的调度器实例。*/

/*
 * sched_due_after：计算“严格晚于 now 的下一个到期点”。
 *
 * 参数：
 *   now    —— 当前毫秒；
 *   period —— 周期，必须非 0（调用方保证）；
 *   phase  —— 相位，必须小于 period（表校验保证）。
*/
static uint32_t sched_due_after(uint32_t now, uint16_t period, uint16_t phase)
{
    uint32_t base;
    uint32_t due;

    base = now - (now % period);
    due = base + (uint32_t)phase;
    if (due <= now)
    {
        due += period;
    }
    return due;
}

/* 任务表不合法。这个钩子是致命的，调用之后停住。 */
static void sched_fail_table(uint32_t index, uint32_t detail)
{
    (void)plat_raise(&plat_hook_sched_bad_table, index, detail);
}

/*检查一个任务描述是否合法。*/
static void sched_check_task(const sched_task_t *task, uint8_t index)
{
    if (task->period_ms == 0u)
    {
        /* 只做初始化的任务：相位必须写 0。 */
        if (task->phase_ms != 0u)
        {
            sched_fail_table(index, task->phase_ms);
        }
        return;
    }
    /* 周期任务：相位必须小于周期，并且必须有 on_tick。 */
    if (task->phase_ms >= task->period_ms || task->pf_on_tick == NULL)
    {
        sched_fail_table(index, task->phase_ms);
    }
}

/*
 * sched_round_try：检查本轮是否所有启用的周期任务都返回过；是则喂狗并开始新一轮。
 *
 * 参数：
 *   s   —— 调度器；
 *   now —— 当前毫秒（本轮结束时刻）；
 *   err —— 输入/输出：把 kick 的错误合并进本次 poll 的返回值。
 *
 * 统计：
 *   need —— 参与本轮的任务数（period != 0 且 enabled != 0）；
 *   done —— 其中 round_done != 0 的数量。
 *
 * 规则：
 *   - need == 0：没有周期任务，不喂（没人能证明系统还活着）；
 *   - done != need：还有任务没返回，继续等；
 *   - done == need：调用 kick(now - round_start)。
 *
 * kick 之后：
 *   - round_start = now；
 *   - 所有任务的 round_done 清 0（包括 disabled 的，保持干净）。
*/
static void sched_round_try(sched_t *s, uint32_t now, result_t *err)
{
    uint8_t i;
    uint8_t need;
    uint8_t done;

    need = 0u;
    done = 0u;
    for (i = 0u; i < s->count; i++)
    {
        /* 只做初始化的任务、被停用的任务，不参与本轮。 */
        if (s->table[i].period_ms == 0u || s->rt[i].enabled == 0u)
        {
            continue;
        }
        need++;
        if (s->rt[i].round_done != 0u)
        {
            done++;
        }
    }
    /* 没有周期任务，或者还有任务没返回：这一轮还没结束。 */
    if (need == 0u || done != need)
    {
        return;
    }
    /* 本轮完成：喂狗。kick 可以为 NULL（表示不喂）。 */
    if (s->kick != NULL)
    {
        result_t kr;

        kr = s->kick(now - s->round_start);
        if (*err == RES_OK && kr != RES_OK)
        {
            *err = kr;
        }
    }
    /* 开始新一轮。 */
    s->round_start = now;
    for (i = 0u; i < s->count; i++)
    {
        s->rt[i].round_done = 0u;
    }
}

/*
 * sched_round_mark：给一个刚返回的周期任务打上“本轮完成”，然后尝试结束本轮。
 *
 * 参数：
 *   s     —— 调度器；
 *   index —— 刚返回的任务编号；
 *   now   —— 当前毫秒；
 *   err   —— 错误合并。
*/
static void sched_round_mark(sched_t *s, uint8_t index, uint32_t now, result_t *err)
{
    if (s->rt[index].enabled != 0u)
    {
        s->rt[index].round_done = 1u;
    }
    sched_round_try(s, now, err);
}

/*
 * sched_run_enter：在调用任务函数之前，记录“谁在跑、从什么时候开始跑”。
 *
 * 顺序很重要：
 *   1) 先写 run_since；
 *   2) 编译器屏障；
 *   3) 再写 run_id。
*/
static void sched_run_enter(sched_t *s, uint8_t index, uint32_t now)
{
    s->run_since = (s->now_ms != NULL) ? s->now_ms() : now;
    __asm__ volatile("" ::: "memory");
    s->run_id = index;
}

/*任务函数返回后，退出“正在跑”状态，并撤销卡死记录。任务真的不返回时，这里走不到。 */
static void sched_run_leave(sched_t *s, uint8_t index)
{
    s->run_id = SCHED_RUN_NONE;
    __asm__ volatile("" ::: "memory");
    if (s->stuck_noted == 0u)
    {
        return;
    }
    s->stuck_noted = 0u;
    if (s->on_stuck != NULL)
    {
        s->on_stuck(index, 0u, 0u);
    }
}

/*
    * sched_run：调用一个任务的 on_tick，并测耗时、检查超时、喂狗。
    *
    * 参数：
    *   s        —— 调度器；
    *   index    —— 任务编号；
    *   event    —— 事件码（0 表示周期到了）；
    *   now      —— 当前毫秒；
    *   periodic —— true：这是周期任务；false：这是事件驱动。
    *
    * 返回：
    *   RES_OK       成功；
    *   RES_OVERRUN  这次调用花的时间超过了任务周期；
    *   其他         任务自己的返回值。
*/
static result_t sched_run(sched_t *s, uint8_t index, uint32_t event, uint32_t now, bool periodic)
{
     /* 任务描述和运行时状态。 */
    const sched_task_t *task;
    sched_task_rt_t *rt;

    /* 任务函数的返回值。 */
    result_t rc;

    /* 距离上次周期运行过了多少毫秒。 */
    uint32_t elapsed;

    /* 测时起点（微秒）。没开测时保持 0。 */
    uint32_t t0;

    /* 本次耗时（微秒）。 */
    uint32_t cost;

    task = &s->table[index];
    rt = &s->rt[index];

    /* 无符号减法，跨越 32 位回绕也正确。 */
    elapsed = now - rt->last_run;

    t0 = 0u;
    if (s->now_us != NULL)
    {
        t0 = s->now_us();
    }

    /* 进入任务：记录开始时刻和任务编号。 */
    sched_run_enter(s, index, now);

    /* 真正调用任务函数。
     * 参数：毫秒快照、距离上次运行的毫秒数、事件码（0=周期）。 */
    rc = task->pf_on_tick(now, elapsed, event);

    /* 退出任务：先声明不在任务里，再撤销可能的卡死记录。 */
    sched_run_leave(s, index);

    /* 先清错误，后面按测时/返回值重新填。 */
    rt->last_err = RES_OK;

    if (s->now_us != NULL)
    {
        /* 本次耗时。now_us 是 32 位微秒，约 71.6 分钟回绕；
         * 单次任务耗时远小于这个范围，无符号减法足够。 */
        cost = s->now_us() - t0;
        rt->last_us = cost;
        if (cost > rt->max_us)
        {
            rt->max_us = cost;
        }

        /* 打印钩子可以为 NULL；返回值忽略。 */
        if (s->print_task != NULL)
        {
            (void)s->print_task(task->name, cost);
        }

        /* 超周期判断：cost 单位微秒，period 单位毫秒，所以乘 1000。
         * period == 0 的任务不参与（它没有周期）。 */
        if (task->period_ms != 0u && cost > ((uint32_t)task->period_ms * 1000u))
        {
            rt->last_err = RES_OVERRUN;
            (void)plat_raise(&plat_hook_sched_overrun, index, cost);

            /* 如果任务本身返回 OK，把返回值升级成 OVERRUN；
             * 如果任务已经返回了别的错误，保留那个错误。 */
            if (rc == RES_OK)
            {
                rc = RES_OVERRUN;
            }
        }
    }

    /* 任务自己返回了非 OK，并且不是因为超周期：
     * 记录错误并触发 task_fail（非致命）。 */
    if (rc != RES_OK && rt->last_err != RES_OVERRUN)
    {
        rt->last_err = (int32_t)rc;
        (void)plat_raise(&plat_hook_task_fail, index, (uint32_t)rc);
    }

    /* 只有周期运行才更新到期时间；事件插队不改。
     * 下一次到期仍然按“网格 + 相位”重新算。 */
    if (periodic)
    {
        rt->last_run = now;
        rt->next_due = sched_due_after(now, task->period_ms, task->phase_ms);
    }

    return rc;
}

/*
 * sched_events：从事件邮箱取事件，并立刻执行目标任务的 on_tick。
 *
 * 参数：
 *   s   —— 调度器；
 *   now —— 本次 poll 的毫秒快照。
 *
 * 返回：
 *   本次处理中遇到的第一个错误（RES_OK 表示都正常）。
*/
static result_t sched_events(sched_t *s, uint32_t now)
{
    result_t err;
    uint8_t n;
    uint8_t id;
    uint32_t event;

    err = RES_OK;
    n = 0u;

    while (n < SCHED_EVT_BURST && evt_sw_driver.pf_take(&id, &event))
    {
        result_t rc;

        n++;

        /* 编号越界，或目标任务没有 on_tick：记录一次任务失败。
         * 事件已经被 evt_take 取出并清掉，不能放回去。 */
        if (id >= s->count || s->table[id].pf_on_tick == NULL)
        {
            (void)plat_raise(&plat_hook_task_fail, id, event);
            if (err == RES_OK)
            {
                err = RES_PARAM;
            }
            continue;
        }

        /* 目标任务已停用：事件丢弃，不执行。 */
        if (s->rt[id].enabled == 0u)
        {
            continue;
        }

        /* 插队执行：periodic=false，不改目标任务的到期时间。 */
        rc = sched_run(s, id, event, now, false);
        if (err == RES_OK && rc != RES_OK)
        {
            err = rc;
        }
    }

    return err;
}

/*初始化调度器*/
static result_t sched_init(sched_t *s, const sched_task_t *table, sched_task_rt_t *rt, uint8_t count,
                           sched_now_ms_fn now_ms, sched_now_us_fn now_us, sched_print_fn print_task,
                           sched_kick_fn kick, sched_wait_fn wait, sched_stuck_fn on_stuck)
{
    uint32_t now;
    uint8_t i;
    result_t err;

    /* ---- 参数检查：对象和取时间函数是硬性要求 ---- */
    if (s == NULL || now_ms == NULL)
    {
        sched_fail_table(0u, 0u);
        return RES_PARAM;
    }

    /* ---- 参数检查：有任务时，表和运行时数组必须存在 ---- */
    if (count > 0u && (table == NULL || rt == NULL))
    {
        sched_fail_table(0u, count);
        return RES_PARAM;
    }

    /* ---- 校验每个任务描述 ----
     * 非法时 sched_fail_table 是致命的，不会返回。 */
    for (i = 0u; i < count; i++)
    {
        sched_check_task(&table[i], i);
    }

    /* ---- 用同一个时间基准初始化所有任务 ---- */
    now = now_ms();
    for (i = 0u; i < count; i++)
    {
        rt[i].last_run = now;
        rt[i].last_us = 0u;
        rt[i].max_us = 0u;
        rt[i].last_err = RES_OK;
        rt[i].round_done = 0u;
        rt[i].enabled = 1u;

        if (table[i].period_ms == 0u)
        {
            /* 只做初始化的任务：next_due 不使用，写 0 表示“没有周期”。 */
            rt[i].next_due = 0u;
        }
        else
        {
            /* 周期任务：第一次到期严格晚于 now，落在网格 + 相位上。 */
            rt[i].next_due = sched_due_after(now, table[i].period_ms, table[i].phase_ms);
        }
    }

    /* ---- 保存任务表和注入的函数 ---- */
    s->table = table;
    s->rt = rt;
    s->count = count;
    s->now_ms = now_ms;
    s->now_us = now_us;
    s->print_task = print_task;
    s->kick = kick;
    s->wait = wait;
    s->on_stuck = on_stuck;

    /* ---- 初始化调度器状态 ---- */
    s->last_ms = now;
    s->round_start = now;
    s->run_since = 0u;
    s->run_id = SCHED_RUN_NONE;
    s->stuck_noted = 0u;
    s->in_poll = 0u;
    s->ready = 1u;

    /* ---- 单实例绑定：中断和 pf_enable 通过它找到这个实例 ---- */
    s_bound = s;

    /* ---- 按表顺序调用每个任务的初始化 ---- */
    err = RES_OK;
    for (i = 0u; i < count; i++)
    {
        result_t rc;

        if (table[i].pf_init == NULL)
        {
            continue;
        }

        rc = table[i].pf_init();
        if (rc != RES_OK)
        {
            (void)plat_raise(&plat_hook_task_fail, i, (uint32_t)rc);
            if (err == RES_OK)
            {
                err = rc;
            }
        }
    }

    return err;
}

/*轮询调用 新的一毫秒先发事件，再跑到期任务，每个任务返回后再发一次。毫秒没变就只发事件然后调用 wait。 */
static result_t sched_poll(sched_t *s)
{
    result_t err;
    uint32_t now;
    uint8_t i;

    /* ---- 对象/初始化检查 ---- */
    if (s == NULL || s->ready == 0u)
    {
        return RES_PARAM;
    }

    /* ---- 重入检查 ----
     * 任务函数里又调用了一次 pf_poll（或 app_loop）会走到这里。
     * 继续跑可能递归/栈溢出，所以是致命异常。 */
    if (s->in_poll != 0u)
    {
        (void)plat_raise(&plat_hook_sched_reenter, 0u, 0u);
        return RES_STATE;
    }
    s->in_poll = 1u;

    /* ---- 读当前毫秒 ---- */
    now = s->now_ms();

    if (now == s->last_ms)
    {
        /* 这一毫秒已经处理过了。
         * 先把邮箱里的事件发出去，再调用 wait。
         * 清 in_poll 必须在 wait 之前，原因见函数头注释。 */
        err = sched_events(s, now);
        s->in_poll = 0u;
        if (s->wait != NULL)
        {
            s->wait();
        }
        return err;
    }

    /* ---- 新的一毫秒 ---- */
    s->last_ms = now;

    /* 先处理已经在邮箱里的事件。 */
    err = sched_events(s, now);

    /* ---- 按表顺序跑到期任务 ---- */
    for (i = 0u; i < s->count; i++)
    {
        result_t rc;
        result_t er;

        /* 只做初始化、没有入口、已停用：跳过。 */
        if (s->table[i].period_ms == 0u || s->table[i].pf_on_tick == NULL || s->rt[i].enabled == 0u)
        {
            continue;
        }

        /* 到期判断：
         * (now - next_due) 的最高位为 1 表示 now 还没到 next_due。
         * 用无符号减法，跨越 32 位回绕也正确。 */
        if ((uint32_t)(now - s->rt[i].next_due) >= 0x80000000u)
        {
            continue;
        }

        /* 周期运行：event=0，periodic=true。 */
        rc = sched_run(s, i, 0u, now, true);
        if (err == RES_OK && rc != RES_OK)
        {
            err = rc;
        }

        /* 打本轮完成标记；如果这是最后一个未完成任务，会在这里喂狗。 */
        sched_round_mark(s, i, now, &err);

        /* 这个任务可能刚发了事件，立刻处理。 */
        er = sched_events(s, now);
        if (err == RES_OK && er != RES_OK)
        {
            err = er;
        }
    }

    /* 防御性再试一次结束本轮（通常已经由 round_mark 处理）。 */
    sched_round_try(s, now, &err);

    s->in_poll = 0u;
    return err;
}

/* （节拍中断里的卡死检测）节拍中断每毫秒进来一次。只在任务还没返回、并且超过它自己的周期时记一次。 */
static void sched_on_ms(uint32_t now)
{
    sched_t *s;
    uint8_t id;
    uint32_t elapsed;
    uint32_t limit;

    s = s_bound;
    if (s == NULL || s->ready == 0u || s->table == NULL)
    {
        return;
    }

    id = s->run_id;

    /* 不在任务里（255）、编号越界、已经记过：都不处理。 */
    if (id == SCHED_RUN_NONE || id >= s->count || s->stuck_noted != 0u)
    {
        return;
    }

    elapsed = now - s->run_since;

    limit = s->table[id].period_ms;
    if (limit == 0u)
    {
        limit = SCHED_STUCK_FALLBACK_MS;
    }

    /* 还没超过自己的周期：正常。 */
    if (elapsed <= limit)
    {
        return;
    }

    /* 超过：只记一次。 */
    s->stuck_noted = 1u;
    if (s->on_stuck != NULL)
    {
        s->on_stuck(id, elapsed, 1u);
    }
}

/*
 * sched_enable：停止/恢复一个任务。
 *
 * 参数：
 *   task_id —— 任务表下标；
 *   enable  —— 0 停止，非 0 恢复。
 *
 * 返回：
 *   RES_STATE  调度器还没初始化，或运行时数组为空；
 *   RES_PARAM  任务编号越界；
 *   RES_OK     设置成功。
*/
static result_t sched_enable(uint8_t task_id, uint8_t enable)
{
    if (s_bound == NULL || s_bound->ready == 0u || s_bound->rt == NULL)
    {
        return RES_STATE;
    }
    if (task_id >= s_bound->count)
    {
        return RES_PARAM;
    }
    s_bound->rt[task_id].enabled = (enable != 0u) ? 1u : 0u;
    return RES_OK;
}

const sched_driver_interface_t sched_sw_driver =
{
    .pf_init   = sched_init,
    .pf_poll   = sched_poll,
    .pf_on_ms  = sched_on_ms,
    .pf_enable = sched_enable,
};
