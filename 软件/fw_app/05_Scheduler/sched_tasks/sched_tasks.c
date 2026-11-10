#include "sched_tasks.h"
#include "config.h"
#include "log.h"
#include "app_heartbeat.h"
#include <stddef.h>

#if (SCHED_ID_LOG != 0u) || (SCHED_ID_HEARTBEAT != 1u)
#error "task id macros must match the table order below"
#endif

/*日志任务的初始化*/
static result_t log_task_init(void)
{
    return log_sw_driver.pf_init();
}

/*把日志组件的返回值翻译成“调度器能接受的任务返回值*/
static result_t log_task_finish(result_t rc)
{
    if (rc == RES_STATE)
    {
        return RES_OK;
    }
    return rc;
}

/*
 * log_task_on_tick：日志任务的周期/事件入口。
 *
 * 参数：
 *   now_ms     —— 当前毫秒（这里不用）；
 *   elapsed_ms —— 距上次运行过了多久（这里不用）；
 *   event      —— 0：周期到了；
 *                 非 0：某个任务发来的事件。
*/
static result_t log_task_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event)
{
    (void)now_ms;
    (void)elapsed_ms;
    if (event == EVT_LOG_EMERGENCY)
    {
        return log_task_finish(log_sw_driver.pf_flush_emergency());
    }
    return log_task_finish(log_sw_driver.pf_flush_daily());
}
/*
 * sched_tasks.c 任务表
 *   name      任务名，调试用，调度器不做逻辑判断；
 *   period_ms 周期（毫秒），0 表示只做初始化；
 *   phase_ms  相位（毫秒），必须小于周期；周期为 0 时必须为 0；
 *   pf_init   初始化函数，允许 NULL；
 *   pf_on_tick 周期/事件入口，周期非 0 时必须非 NULL。
*/
static const sched_task_t s_tasks[] =
{
    {
        .name       = "log",
        .period_ms  = SCHED_PERIOD_LOG_MS,
        .phase_ms   = SCHED_PHASE_LOG_MS,
        .pf_init    = log_task_init,
        .pf_on_tick = log_task_on_tick,
    },
    {
        .name       = "heartbeat",
        .period_ms  = SCHED_PERIOD_HEARTBEAT_MS,
        .phase_ms   = SCHED_PHASE_HEARTBEAT_MS,
        .pf_init    = app_heartbeat_init,
        .pf_on_tick = app_heartbeat_on_tick,
    },
};

static sched_task_rt_t s_rt[SCHED_TASK_MAX];/*任务运行时数组*/

typedef char sched_tasks_fit_check[(sizeof(s_tasks) / sizeof(s_tasks[0]) == SCHED_ID_COUNT) ? 1 : -1];/*表行数必须等于任务编号总数 SCHED_ID_COUNT*/
typedef char sched_tasks_room_check[(sizeof(s_tasks) / sizeof(s_tasks[0]) <= SCHED_TASK_MAX) ? 1 : -1];/*表行数不能超过运行时数组容量 SCHED_TASK_MAX*/

/*
 * sched_tasks_bind：输出任务表、运行时数组、任务个数。
 *
 * 参数：
 *   table —— 输出任务表指针；不需要可传 NULL；
 *   rt    —— 输出运行时数组指针；不需要可传 NULL；
 *   count —— 输出任务个数；不需要可传 NULL。
*/
void sched_tasks_bind(const sched_task_t **table, sched_task_rt_t **rt, uint8_t *count)
{
    (void)sizeof(sched_tasks_fit_check);
    (void)sizeof(sched_tasks_room_check);
    if (table != NULL)
    {
        *table = s_tasks;
    }
    if (rt != NULL)
    {
        *rt = s_rt;
    }
    if (count != NULL)
    {
        *count = (uint8_t)(sizeof(s_tasks) / sizeof(s_tasks[0]));
    }
}
