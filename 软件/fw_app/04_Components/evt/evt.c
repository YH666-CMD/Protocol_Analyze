#include "evt.h"
#include "config.h"
#include <stddef.h>

typedef struct
{
    uint8_t  pending;  /* 0 空，1 有未处理事件 */
    uint32_t event;    /* 事件码；EVT_NONE(0) 表示没有事件 */
} evt_box_t;

static evt_box_t   s_box[SCHED_TASK_MAX];     /*s_box：每个任务一个邮箱，下标就是任务编号*/
static uint8_t     s_order[SCHED_TASK_MAX];   /*按“发送先后”记录哪些任务的邮箱里有事件*/
static uint8_t     s_head;         /*下一个要写的 s_order 位置*/
static uint8_t     s_tail;         /*下一个要读的 s_order 位置*/
static uint8_t     s_queued;       /*环里当前有多少条待取事件*/
static uint8_t     s_count;        /*有效任务个数*/
static evt_lock_fn s_lock;
static evt_unlock_fn s_unlock;

/*进入临界区 */
static uint32_t evt_enter(void)
{
    if (s_lock == NULL)
    {
        return 0u;
    }
    return s_lock();
}

/*退出临界区*/
static void evt_leave(uint32_t state)
{
    if (s_unlock != NULL)
    {
        s_unlock(state);
    }
}

/* 清掉全部邮箱 */
static result_t evt_init(uint8_t task_count, evt_lock_fn lock, evt_unlock_fn unlock)
{
    uint8_t i;

    if (task_count == 0u || task_count > SCHED_TASK_MAX)
    {
        return RES_PARAM;
    }
    if ((lock == NULL) != (unlock == NULL))
    {
        return RES_PARAM;
    }

    for (i = 0u; i < SCHED_TASK_MAX; i++)
    {
        s_box[i].pending = 0u;
        s_box[i].event = EVT_NONE;
    }
    s_head = 0u;
    s_tail = 0u;
    s_queued = 0u;
    s_count = task_count;
    s_lock = lock;
    s_unlock = unlock;
    return RES_OK;
}

/*把事件放进目标任务的邮箱*/
static result_t evt_send(uint8_t task_id, uint32_t event)
{
    uint32_t state;
    result_t rc;

    if (task_id >= s_count || event == EVT_NONE)
    {
        return RES_PARAM;
    }

    state = evt_enter();
    if (s_box[task_id].pending != 0u || s_queued >= SCHED_TASK_MAX)
    {
        rc = RES_NO_MEM;
    }
    else
    {
        s_box[task_id].event = event;
        s_box[task_id].pending = 1u;
        s_order[s_head] = task_id;
        s_head++;
        if (s_head >= SCHED_TASK_MAX)
        {
            s_head = 0u;
        }
        s_queued++;
        rc = RES_OK;
    }
    evt_leave(state);
    if (rc != RES_OK)
    {
        (void)plat_raise(&plat_hook_evt_full, task_id, event);
    }
    return rc;
}

/*取出事件*/
static bool evt_take(uint8_t *task_id, uint32_t *event)
{
    uint32_t state;
    uint8_t id;
    bool got;

    if (task_id == NULL || event == NULL)
    {
        return false;
    }

    state = evt_enter();
    if (s_queued == 0u)
    {
        got = false;
    }
    else
    {
        id = s_order[s_tail];
        s_tail++;
        if (s_tail >= SCHED_TASK_MAX)
        {
            s_tail = 0u;
        }
        s_queued--;
        *task_id = id;
        *event = s_box[id].event;
        s_box[id].pending = 0u;
        s_box[id].event = EVT_NONE;
        got = true;
    }
    evt_leave(state);
    return got;
}

const evt_driver_t evt_sw_driver =
{
    .pf_init = evt_init,
    .pf_send = evt_send,
    .pf_take = evt_take,
};
