#include "sched_tasks.h"
#include "app_heartbeat.h"
#include "config.h"
#include <stddef.h>

static const sched_task_t s_tasks[] =
{
    {
        .name      = "heartbeat",
        .period_ms = SCHED_PERIOD_HEARTBEAT_MS,
        .phase_ms  = SCHED_PHASE_HEARTBEAT_MS,
        .pf_init   = app_heartbeat_init,
        .pf_on_tick = app_heartbeat_on_tick,
    },
};

static sched_task_rt_t s_rt[SCHED_TASK_MAX];

typedef char sched_tasks_fit_check[(sizeof(s_tasks) / sizeof(s_tasks[0]) <= SCHED_TASK_MAX) ? 1 : -1];

void sched_tasks_bind(const sched_task_t **table, sched_task_rt_t **rt, uint8_t *count)
{
    (void)sizeof(sched_tasks_fit_check);

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
