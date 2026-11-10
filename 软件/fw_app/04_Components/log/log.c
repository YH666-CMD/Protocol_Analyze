#include "log.h"
#include "config.h"
/*
 *   日常日志：
 *     业务任务 -> pf_daily() -> 追加到 RAM 数组 s_daily[]
 *     日志任务（每 100 ms）-> pf_flush_daily() -> 尝试写 Flash
 *
 *   紧急日志：
 *     看门狗钩子 -> pf_emergency() -> 写 g_log_emergency（只写 RAM）
 *     节拍中断   -> pf_stuck()     -> 写 g_log_emergency（只写 RAM）
 *     上电       -> pf_boot()      -> 如果 flag 有效，尝试写 Flash
 *     日志任务收到 EVT_LOG_EMERGENCY -> pf_flush_emergency()
 */
typedef struct
{
    uint8_t  module;/*谁记的（模块号）*/
    uint8_t  event; /*发生了什么（事件号）*/
    uint16_t arg;   /*一个 16 位附加参数*/
} log_daily_t;

static log_daily_t s_daily[LOG_DAILY_DEPTH];/*日常日志的 RAM 数组*/
static uint8_t s_daily_count;               /*当前已经攒了多少条*/
/*全局唯一的一条紧急记录*/
volatile log_emergency_t g_log_emergency;   /*定义在 .bss，上电由启动代码清零,现在它不在 NOINIT 段，复位后会被清零*/

/*Flash 写入占位*/
static result_t flash_write_daily(void)
{
    return RES_STATE;
}

/*把 g_log_emergency 写进 Flash*/
static result_t flash_write_emergency(void)
{
    return RES_STATE;
}

/* 上电第一件事。标志有效才去碰 FLASH */
static result_t log_boot(void)
{
    if (g_log_emergency.flag != LOG_EMERGENCY_FLAG) /* 没有待写记录：不碰 Flash，直接成功。 */
    {
        return RES_OK;
    }
    return log_sw_driver.pf_flush_emergency();/* 有待写记录：走统一的紧急 flush 流程。 */
}

/* 清日常队列 */
static result_t log_init(void)
{
    uint8_t i;

    for (i = 0u; i < LOG_DAILY_DEPTH; i++)
    {
        s_daily[i].module = 0u;
        s_daily[i].event = 0u;
        s_daily[i].arg = 0u;
    }
    s_daily_count = 0u;
    return RES_OK;
}

/* 追加一条日常记录。满了就停，旧的还在 */
static result_t log_daily(uint8_t module, uint8_t event, uint16_t arg)
{
    if (s_daily_count >= LOG_DAILY_DEPTH)
    {
        return RES_NO_MEM;
    }
    s_daily[s_daily_count].module = module;
    s_daily[s_daily_count].event = event;
    s_daily[s_daily_count].arg = arg;
    s_daily_count++;
    return RES_OK;
}

/*写一条紧急记录（只写 RAM，不碰 Flash）*/
static result_t log_emergency(uint32_t code, uint32_t extra)
{
    g_log_emergency.code = code;
    g_log_emergency.extra = extra;
    g_log_emergency.module = 0u;
    g_log_emergency.arg = 0u;
    g_log_emergency.flag = LOG_EMERGENCY_FLAG;
    return RES_OK;
}

/*记录“某个任务超过自己的周期还没返回*/
static result_t log_stuck(uint8_t task_id, uint32_t elapsed_ms)
{
    /* 已有别的紧急记录：保留它，不覆盖。 */
    if (g_log_emergency.flag == LOG_EMERGENCY_FLAG &&
        g_log_emergency.code != (uint32_t)PLAT_ERR_SCHED_STUCK)
    {
        return RES_BUSY;
    }
    /* 写卡住现场。arg 截断，extra 保留完整毫秒。 */
    g_log_emergency.module = task_id;
    g_log_emergency.arg = (elapsed_ms > 0xFFFFu) ? 0xFFFFu : (uint16_t)elapsed_ms;
    g_log_emergency.extra = elapsed_ms;
    g_log_emergency.code = (uint32_t)PLAT_ERR_SCHED_STUCK;
    /* 标志最后写。 */
    g_log_emergency.flag = LOG_EMERGENCY_FLAG;
    return RES_OK;
}

/*任务后来返回了，撤掉“卡住”这条记录*/
static result_t log_stuck_clear(void)
{
    if (g_log_emergency.flag == LOG_EMERGENCY_FLAG &&
        g_log_emergency.code == (uint32_t)PLAT_ERR_SCHED_STUCK)
    {
        g_log_emergency.flag = 0u;
    }
    return RES_OK;
}

/*尝试把日常 RAM 记录写进 Flash*/
static result_t log_flush_daily(void)
{
    result_t rc;

    if (s_daily_count == 0u)
    {
        return RES_OK;
    }
    rc = flash_write_daily();
    if (rc == RES_OK)
    {
        s_daily_count = 0u;
        return RES_OK;
    }
    if (rc != RES_STATE)
    {
        (void)plat_raise(&plat_hook_log_flash, s_daily_count, (uint32_t)rc);
    }
    return rc;
}

/*尝试把紧急 RAM 记录写进 Flash*/
static result_t log_flush_emergency(void)
{
    result_t rc;

    if (g_log_emergency.flag != LOG_EMERGENCY_FLAG)
    {
        return RES_OK;
    }
    rc = flash_write_emergency();
    if (rc == RES_OK)
    {
        g_log_emergency.flag = 0u;
        return RES_OK;
    }
    if (rc != RES_STATE)
    {
        (void)plat_raise(&plat_hook_log_flash, g_log_emergency.code, (uint32_t)rc);
    }
    return rc;
}

const log_driver_t log_sw_driver =
{
    .pf_boot            = log_boot,
    .pf_init            = log_init,
    .pf_daily           = log_daily,
    .pf_emergency       = log_emergency,
    .pf_stuck           = log_stuck,
    .pf_stuck_clear     = log_stuck_clear,
    .pf_flush_daily     = log_flush_daily,
    .pf_flush_emergency = log_flush_emergency,
};
