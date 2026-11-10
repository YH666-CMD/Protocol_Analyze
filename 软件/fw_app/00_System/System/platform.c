#include "platform.h"
#include "config.h"
#include "log.h"
#include "evt.h"
#include <stddef.h>

volatile plat_fault_info_t g_plat_last_fault;
volatile uint32_t g_plat_reset_reason;

/* 致命异常停在这里。空循环要留住（或直接重启），不能被优化掉。 */
static void plat_halt(void)
{
    for (;;)
    {
        __asm__ volatile("" ::: "memory");
    }
}

/* 上电读到复位原因后进入这里。每种原因单独返回，现在不做额外动作。 */
int32_t plat_on_reset(uint32_t reason)
{
    g_plat_reset_reason = reason;
    switch (reason)
    {
    case PLAT_RESET_PIN:
        return PLAT_OK;
    case PLAT_RESET_POR:    /*可以用来清标志位*/
        return PLAT_OK;
    case PLAT_RESET_SOFTWARE:
        return PLAT_OK;
    case PLAT_RESET_IWDG:   /*可以用来记录日志*/
        return PLAT_OK;
    case PLAT_RESET_WWDG:
        return PLAT_OK;
    case PLAT_RESET_LOWPOWER:
        return PLAT_OK;
    case PLAT_RESET_UNKNOWN:
        return PLAT_OK;
    default:
        return PLAT_OK;
    }
}

/* 任务超过周期还没返回。这是在中断里！只写紧急 RAM。任务若又返回了，把这条记录撤掉，别的紧急记录不动。 */
void plat_on_task_stuck(uint8_t task_id, uint32_t elapsed_ms, uint8_t active)
{
    if (active == 0u)
    {
        (void)log_sw_driver.pf_stuck_clear();
        return;
    }
    (void)log_sw_driver.pf_stuck(task_id, elapsed_ms);
}

/* 所有异常的公共入口 */
int32_t plat_raise(const plat_hook_t *hook, uint32_t arg0, uint32_t arg1)
{
    int32_t rc; 
    if (hook == NULL || hook->fn == NULL)/*没有钩子可调用*/
    {
        g_plat_last_fault.name = "null_hook";
        g_plat_last_fault.arg0 = arg0;
        g_plat_last_fault.arg1 = arg1;
        g_plat_last_fault.result = PLAT_ERR_SCHED_BAD_TABLE;
        plat_halt();
        return PLAT_ERR_SCHED_BAD_TABLE;
    }
    /*记录现场*/
    g_plat_last_fault.name = hook->name;
    g_plat_last_fault.arg0 = arg0;
    g_plat_last_fault.arg1 = arg1;
    /*调用钩子函数*/
    rc = hook->fn(arg0, arg1);
    /*记录返回值*/
    g_plat_last_fault.result = rc;
    /*fatal为1的钩子返回后停住代表故障严重*/
    if (hook->fatal != 0u)
    {
        plat_halt();
    }
    /*返回非致命异常钩子返回值*/
    return rc;
}
/*任务表不合法*/
static int32_t on_sched_bad_table(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_SCHED_BAD_TABLE;
}
/*调度重入*/
static int32_t on_sched_reenter(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_SCHED_REENTER;
}
/*任务本次耗时超过自己的周期，只记录因为擦写FLASH可能出现*/
static int32_t on_sched_overrun(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_SCHED_OVERRUN;
}
/*毫秒时基没有走*/
static int32_t on_timebase_dead(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_TIMEBASE;
}

/* 看门狗通知到这里。先把现场写进紧急 RAM，再发事件给日志任务。 */
static int32_t on_wdt_hang(uint32_t arg0, uint32_t arg1)
{
    (void)log_sw_driver.pf_emergency(arg0, arg1);/*arg0用掉的毫秒值，arg1额外信息*/
    (void)evt_sw_driver.pf_send(SCHED_ID_LOG, EVT_LOG_EMERGENCY);
    return PLAT_ERR_WDT_HANG;
}
/*目标任务邮箱里还有没处理的事件，新事件被丢弃*/
static int32_t on_evt_full(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_EVT_FULL;
}
/*日志写 Flash 失败*/
static int32_t on_log_flash(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_LOG_FLASH;
}
/*任务函数返回了非 RES_OK*/
static int32_t on_task_fail(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return PLAT_ERR_TASK;
}
/*钩子对象表*/
/*每个对象三个字段：名字、处理函数、是否致命*/
const plat_hook_t plat_hook_sched_bad_table = { "sched_bad_table", on_sched_bad_table, 1u };
const plat_hook_t plat_hook_sched_reenter   = { "sched_reenter",   on_sched_reenter,   1u };
const plat_hook_t plat_hook_sched_overrun   = { "sched_overrun",   on_sched_overrun,   0u };
const plat_hook_t plat_hook_timebase_dead   = { "timebase_dead",   on_timebase_dead,   1u };
const plat_hook_t plat_hook_wdt_hang        = { "wdt_hang",        on_wdt_hang,        0u };
const plat_hook_t plat_hook_evt_full        = { "evt_full",        on_evt_full,        0u };
const plat_hook_t plat_hook_log_flash       = { "log_flash",       on_log_flash,       0u };
const plat_hook_t plat_hook_task_fail       = { "task_fail",       on_task_fail,       0u };
