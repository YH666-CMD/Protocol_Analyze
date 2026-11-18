#include "log.h"
#include "config.h"
#include "crc.h"
#include "bsp_flash.h"
#include "bsp_time.h"
#include "ota_boot.h"
#include <stddef.h>
/*
 *   日常日志：
 *     业务任务 -> pf_daily() -> 追加到 RAM 数组 s_daily[]
 *     日志任务（每 100 ms）-> pf_flush_daily() -> 收进页缓冲，满页或等满 2 秒再写 Flash
 *
 *   紧急日志：
 *     看门狗钩子 -> pf_emergency() -> 写 g_log_emergency（只写 RAM）
 *     节拍中断   -> pf_stuck()     -> 写 g_log_emergency（只写 RAM）
 *     上电       -> pf_boot()      -> 标志有效就立刻写入日志区，并清掉标志
 *     日志任务收到 EVT_LOG_EMERGENCY -> pf_flush_emergency()
 */
#define LOG_BASE        (MEM_FLASH_PHY_BASE + MEM_LOG_OFS)
#define LOG_NUM         (MEM_LOG_SIZE / sizeof(log_rec_t))
#define LOG_PER_PAGE    (MEM_FLASH_PAGE / sizeof(log_rec_t))
#define LOG_HOLD_MS     2000u

typedef char log_page_check[((MEM_FLASH_PAGE % sizeof(log_rec_t)) == 0u) ? 1 : -1];

typedef struct
{
    uint8_t  module;/*谁记的（模块号）*/
    uint8_t  event; /*发生了什么（事件号）*/
    uint16_t arg;   /*一个 16 位附加参数*/
} log_daily_t;

static log_daily_t s_daily[LOG_DAILY_DEPTH];/*日常日志的 RAM 数组*/
static uint8_t s_daily_count;               /*当前已经攒了多少条*/
static uint8_t s_taken;                     /*已经收进页缓冲、还没落盘的条数*/
static uint8_t s_fill;                      /*页缓冲里的有效条数*/
static uint8_t s_scanned;
static uint8_t s_boot_pend;                 /* 1 还没收，2 已在页缓冲里 */
static uint8_t s_em_queued;
static uint32_t s_seq;
static uint32_t s_index;
static uint32_t s_latest;
static uint32_t s_hold;
static log_rec_t s_page[LOG_PER_PAGE];
/*全局唯一的一条紧急记录。不赋初值，否则会离开 .share。*/
volatile log_emergency_t g_log_emergency __attribute__((section(".share")));

/*前面各字段按字节相加，再取反码。*/
static uint8_t log_em_digest(void)
{
    uint8_t sum;

    sum = g_log_emergency.flag;
    sum = (uint8_t)(sum + g_log_emergency.module);
    sum = (uint8_t)(sum + (uint8_t)g_log_emergency.arg);
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.arg >> 8));
    sum = (uint8_t)(sum + (uint8_t)g_log_emergency.code);
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.code >> 8));
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.code >> 16));
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.code >> 24));
    sum = (uint8_t)(sum + (uint8_t)g_log_emergency.extra);
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.extra >> 8));
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.extra >> 16));
    sum = (uint8_t)(sum + (uint8_t)(g_log_emergency.extra >> 24));
    return (uint8_t)(~sum);
}

static void log_em_seal(void)
{
    g_log_emergency.check = log_em_digest();
}

/*标志是 0xA5 并且反码对得上，才算有一条待写记录。*/
static uint8_t log_em_ok(void)
{
    return (g_log_emergency.flag == LOG_EMERGENCY_FLAG) &&
           (g_log_emergency.check == log_em_digest());
}

static void log_clear_rec(log_rec_t *rec)
{
    uint8_t *raw;
    uint8_t i;

    raw = (uint8_t *)rec;
    for (i = 0u; i < (uint8_t)sizeof(log_rec_t); i++)
    {
        raw[i] = 0u;
    }
}

static uint8_t log_crc_ok(const log_rec_t *rec)
{
    if (rec->seq == 0u)
    {
        return 0u;
    }
    return (uint8_t)(crc_sw_driver.pf_crc32((const uint8_t *)rec,
                                            (uint32_t)offsetof(log_rec_t, crc)) == rec->crc);
}

/* 找出 seq 最大的一条。没有则 s_latest 为 LOG_NUM，下一次从第 0 页写。 */
static void log_scan(void)
{
    const log_rec_t *rec;
    uint32_t i;
    uint32_t best;

    best = LOG_NUM;
    rec = (const log_rec_t *)LOG_BASE;
    for (i = 0u; i < LOG_NUM; i++)
    {
        if (log_crc_ok(&rec[i]) == 0u)
        {
            continue;
        }
        if ((best == LOG_NUM) || (rec[i].seq > rec[best].seq))
        {
            best = i;
        }
    }
    s_latest = best;
    if (best >= LOG_NUM)
    {
        s_index = 0u;
        s_seq = 1u;
    }
    else
    {
        s_seq = rec[best].seq + 1u;
        s_index = ((best / LOG_PER_PAGE) + 1u) * LOG_PER_PAGE;
        if (s_index >= LOG_NUM)
        {
            s_index = 0u;
        }
    }
    s_scanned = 1u;
}

static void log_ensure(void)
{
    if (s_scanned == 0u)
    {
        log_scan();
    }
}

static result_t log_commit(void)
{
    uint32_t addr;
    uint8_t real;
    uint8_t i;
    result_t rc;

    if (s_fill == 0u)
    {
        return RES_OK;
    }
    real = s_fill;
    for (i = s_fill; i < (uint8_t)LOG_PER_PAGE; i++)
    {
        log_clear_rec(&s_page[i]);
        s_page[i].crc = 1u;
    }
    addr = LOG_BASE + (s_index * sizeof(log_rec_t));
    rc = bsp_flash.pf_erase(addr, MEM_FLASH_PAGE);
    if (rc != RES_OK)
    {
        return rc;
    }
    rc = bsp_flash.pf_write_page(addr, (const uint32_t *)s_page);
    if (rc != RES_OK)
    {
        return rc;
    }
    s_latest = s_index + (uint32_t)real - 1u;
    s_index += LOG_PER_PAGE;
    if (s_index >= LOG_NUM)
    {
        s_index = 0u;
    }
    s_fill = 0u;
    s_em_queued = 0u;
    if (s_boot_pend == 2u)
    {
        s_boot_pend = 0u;
    }
    if (s_taken != 0u)
    {
        for (i = 0u; i < (uint8_t)(s_daily_count - s_taken); i++)
        {
            s_daily[i] = s_daily[s_taken + i];
        }
        s_daily_count = (uint8_t)(s_daily_count - s_taken);
        s_taken = 0u;
    }
    return RES_OK;
}

static void log_push(uint8_t type, uint8_t module, uint8_t event,
                     uint16_t arg, uint32_t code, uint32_t extra)
{
    log_rec_t *rec;

    rec = &s_page[s_fill];
    log_clear_rec(rec);
    rec->seq = s_seq;
    rec->type = type;
    rec->module = module;
    rec->event = event;
    rec->arg = arg;
    rec->code = code;
    rec->extra = extra;
    rec->ms = bsp_time.pf_now_ms();
    rec->crc = crc_sw_driver.pf_crc32((const uint8_t *)rec, (uint32_t)offsetof(log_rec_t, crc));
    if (s_fill == 0u)
    {
        s_hold = rec->ms;
    }
    s_fill++;
    s_seq++;
}

static void log_push_boot(void)
{
    ota_flag_t flag;
    uint8_t trial;
    uint16_t cnt;

    trial = OTA_SLOT_NONE;
    cnt = 0u;
    if (ota_boot.pf_load(&flag) == RES_OK)
    {
        trial = flag.trial_slot;
        cnt = flag.trial_cnt;
    }
    log_push(LOG_KIND_BOOT, (uint8_t)APP_SLOT, trial, cnt, g_plat_reset_reason, 0u);
    s_boot_pend = 2u;
}

static result_t log_write_emergency(void)
{
    result_t rc;

    log_ensure();
    if (s_em_queued == 0u)
    {
        if (s_fill >= (uint8_t)LOG_PER_PAGE)
        {
            rc = log_commit();
            if (rc != RES_OK)
            {
                return rc;
            }
        }
        log_push(LOG_KIND_EMERG, g_log_emergency.module, 0u, g_log_emergency.arg,
                 g_log_emergency.code, g_log_emergency.extra);
        s_em_queued = 1u;
    }
    return log_commit();
}

static void log_copy(uint8_t *dst, const log_rec_t *rec)
{
    const uint8_t *src;
    uint8_t i;

    src = (const uint8_t *)rec;
    for (i = 0u; i < LOG_REC_BYTES; i++)
    {
        dst[i] = src[i];
    }
}

static void log_em_drop(void)
{
    g_log_emergency.flag = 0u;
    log_em_seal();
}

/* 上电第一件事。标志有效才去碰 FLASH */
static result_t log_boot(void)
{
    result_t rc;

    log_scan();
    s_boot_pend = 1u;
    if (log_em_ok() == 0u)
    {
        return RES_OK;
    }
    rc = log_write_emergency();
    if (rc == RES_OK)
    {
        log_em_drop();
    }
    return rc;
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
    s_taken = 0u;
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
    log_em_seal();
    return RES_OK;
}

/*记录“某个任务超过自己的周期还没返回*/
static result_t log_stuck(uint8_t task_id, uint32_t elapsed_ms)
{
    /* 已有别的紧急记录：保留它，不覆盖。 */
    if ((log_em_ok() != 0u) &&
        g_log_emergency.code != (uint32_t)PLAT_ERR_SCHED_STUCK)
    {
        return RES_BUSY;
    }
    /* 写卡住现场。arg 截断，extra 保留完整毫秒。 */
    g_log_emergency.module = task_id;
    g_log_emergency.arg = (elapsed_ms > 0xFFFFu) ? 0xFFFFu : (uint16_t)elapsed_ms;
    g_log_emergency.extra = elapsed_ms;
    g_log_emergency.code = (uint32_t)PLAT_ERR_SCHED_STUCK;
    /* 标志和反码最后写。 */
    g_log_emergency.flag = LOG_EMERGENCY_FLAG;
    log_em_seal();
    return RES_OK;
}

/*任务后来返回了，撤掉“卡住”这条记录*/
static result_t log_stuck_clear(void)
{
    if ((log_em_ok() != 0u) &&
        g_log_emergency.code == (uint32_t)PLAT_ERR_SCHED_STUCK)
    {
        g_log_emergency.flag = 0u;
        log_em_seal();
    }
    return RES_OK;
}

/*尝试把日常 RAM 记录写进 Flash*/
static result_t log_flush_daily(void)
{
    result_t rc;
    uint32_t now;

    log_ensure();
    if ((log_em_ok() != 0u) || (s_em_queued != 0u))
    {
        rc = log_write_emergency();
        if (rc != RES_OK)
        {
            return rc;
        }
        log_em_drop();
    }
    if ((s_boot_pend == 1u) && (s_fill < (uint8_t)LOG_PER_PAGE))
    {
        log_push_boot();
    }
    while ((s_taken < s_daily_count) && (s_fill < (uint8_t)LOG_PER_PAGE))
    {
        log_push(LOG_KIND_DAILY, s_daily[s_taken].module, s_daily[s_taken].event,
                 s_daily[s_taken].arg, 0u, 0u);
        s_taken++;
    }
    if (s_fill == 0u)
    {
        return RES_OK;
    }
    now = bsp_time.pf_now_ms();
    if ((s_fill < (uint8_t)LOG_PER_PAGE) && ((uint32_t)(now - s_hold) < LOG_HOLD_MS))
    {
        return RES_OK;
    }
    rc = log_commit();
    if (rc == RES_OK)
    {
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

    if ((log_em_ok() == 0u) && (s_em_queued == 0u))
    {
        return RES_OK;
    }
    rc = log_write_emergency();
    if (rc == RES_OK)
    {
        log_em_drop();
        return RES_OK;
    }
    if (rc != RES_STATE)
    {
        (void)plat_raise(&plat_hook_log_flash, g_log_emergency.code, (uint32_t)rc);
    }
    return rc;
}

result_t log_read(uint32_t seq, uint8_t *out)
{
    const log_rec_t *rec;
    uint32_t i;

    if (out == 0)
    {
        return RES_PARAM;
    }
    log_ensure();
    if (seq == 0u)
    {
        if (s_fill != 0u)
        {
            log_copy(out, &s_page[s_fill - 1u]);
            return RES_OK;
        }
        if (s_latest >= LOG_NUM)
        {
            return RES_STATE;
        }
        log_copy(out, &((const log_rec_t *)LOG_BASE)[s_latest]);
        return RES_OK;
    }
    for (i = 0u; i < s_fill; i++)
    {
        if (s_page[i].seq == seq)
        {
            log_copy(out, &s_page[i]);
            return RES_OK;
        }
    }
    rec = (const log_rec_t *)LOG_BASE;
    for (i = 0u; i < LOG_NUM; i++)
    {
        if ((log_crc_ok(&rec[i]) != 0u) && (rec[i].seq == seq))
        {
            log_copy(out, &rec[i]);
            return RES_OK;
        }
    }
    return RES_STATE;
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
