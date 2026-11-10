/*
 * sched.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 调度器。状态放在调用方的 sched_t 里，本文件不另做一套全局变量。
 *
 * 约定：
 *   1. 相位不是只加在第一轮。每次周期任务返回后，下一次到期都重新对齐到“周期网格 + 相位”。
 *   2. 任务函数必须返回。不要在里面等待，也不要再调用 pf_poll。
 *      一直不返回时，节拍中断只记下任务编号。复位仍由看门狗完成，这里不切换任务。
 *   3. 事件码 0 表示这次是周期到了。非 0 表示有任务刚把这个事件发给它，调度器会立刻改道去跑。
 *   4. now_us 和 print_task 都传 NULL 时不测耗时。测时打开后，超过本任务周期会返回 RES_OVERRUN。
 *   5. 新任务只要写进任务表，测时和超时检查会跟着跑，不用在任务函数里再包一层。
 *   6. pf_enable 是任务开关。0 停止调度，非 0 恢复。停止期间周期和事件都不会跑这个任务。
 *   7. 毫秒没有变化时，先把已经进邮箱的事件发出去，再调用 wait。wait 为空就不睡。
 *   8. on_stuck 由节拍中断调用，只能记一条记录。任务若随后返回，会再调用一次把记录撤掉。
 */
#ifndef FW_APP_05_SCHEDULER_SCHED_SCHED_H_
#define FW_APP_05_SCHEDULER_SCHED_SCHED_H_

#include <stdint.h>
#include "platform.h"

typedef result_t (*sched_init_fn)(void);  /*任务自己的初始化函数*/
/*
 * sched_tick_fn：任务的周期/事件入口。
 *
 * 参数：
 *   now_ms     —— 本次 pf_poll 读到的毫秒（同一次 poll 内所有任务共用这个快照）；
 *   elapsed_ms —— 距离这个任务上一次“周期运行”过了多少毫秒；
 *   event      —— 0：这次是周期到了；
 *                 非 0：某个任务刚给它发了这个事件，调度器让它插队执行。
 * 返回：
 *   RES_OK     正常；
 *   其他       任务失败，调度器记录并触发 plat_hook_task_fail（非致命）。
*/
typedef result_t (*sched_tick_fn)(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event);
typedef uint32_t (*sched_now_ms_fn)(void);  /*取当前毫秒*/
typedef uint32_t (*sched_now_us_fn)(void);  /*取当前微秒（毫秒×1000 + 本毫秒内走过的微秒）传 NULL 表示不测*/
typedef result_t (*sched_print_fn)(const char *name, uint32_t cost_us);/*每任务耗时输出,参数：任务名、本次花掉的微秒。*/
typedef result_t (*sched_kick_fn)(uint32_t round_ms);                  /*喂狗函数,参数：本轮任务耗时毫秒。*/
typedef void     (*sched_wait_fn)(void);    /*空闲睡眠*/
/*
 * sched_stuck_fn：任务卡死记录。
 * 参数：
 *   task_id    —— 任务编号；
 *   elapsed_ms —— 已经卡了多少毫秒；
 *   active     —— 1：还没返回，写记录；
 *                 0：任务后来返回了，撤掉记录。
*/
typedef void     (*sched_stuck_fn)(uint8_t task_id, uint32_t elapsed_ms, uint8_t active);

typedef struct
{
    const char   *name;       /*任务名*/
    uint16_t      period_ms;  /*周期，单位毫秒*/
    uint16_t      phase_ms;   /*相位，单位毫秒*/
    sched_init_fn pf_init;    /*上电初始化一次，允许 NULL*/
    sched_tick_fn pf_on_tick; /*周期/事件入口*/
} sched_task_t;

typedef struct
{
    uint32_t next_due;      /*下一次到期时刻（毫秒，32 位）*/
    uint32_t last_run;      /*上一次“周期运行”的时刻*/
    uint32_t last_us;       /*最近一次调用花掉的微秒*/
    uint32_t max_us;        /*历史最大耗时（微秒）*/
    int32_t  last_err;      /*最近一次错误*/
    uint8_t  round_done;    /*本轮是否已经返回过 1：本轮已完成；0：本轮还没跑*/
    uint8_t  enabled;       /*任务开关，0 停止调度，非 0 恢复*/
} sched_task_rt_t;

typedef struct
{
    const sched_task_t *table;       /*任务描述表*/
    sched_task_rt_t    *rt;          /*任务运行时数组*/
    uint8_t             count;       /*任务个数*/
    sched_now_ms_fn     now_ms;      /*注入的“取毫秒”函数*/
    sched_now_us_fn     now_us;      /*注入的“取微秒”函数*/
    sched_print_fn      print_task;  /*注入的“打印任务耗时”函数*/
    sched_kick_fn       kick;        /*注入的“按轮喂狗”函数*/
    sched_wait_fn       wait;        /*注入的“空闲睡眠”函数*/
    sched_stuck_fn      on_stuck;    /*注入的“卡死记录”函数*/
    uint32_t            last_ms;     /*上一次 pf_poll 看到的毫秒。用来判断“这一毫秒处理过没有”*/
    uint32_t            round_start; /*本轮开始时刻。kick 的参数 = now - round_start。*/
    volatile uint32_t   run_since;   /* 这次任务调用开始时的毫秒 */
    volatile uint8_t    run_id;      /*当前正在执行的任务编号*/
    volatile uint8_t    stuck_noted; /*这次卡住是否已经记录过1记过0还没记*/
    uint8_t             in_poll;     /*重入保护 正在 pf_poll 里；此时再次进入 pf_poll 会触发致命重入异常*/
    uint8_t             ready;       /*调度器是否已经初始化完成*/
} sched_t;

typedef struct
{
    /* 
     * pf_init：初始化调度器。
     *
     * 形参：
     *   s          —— 调度器对象（调用方提供，生命周期要覆盖整个程序）；
     *   table      —— 任务表；
     *   rt         —— 运行时数组，长度至少 count；
     *   count      —— 任务个数；
     *   now_ms     —— 取毫秒函数，不能为 NULL；
     *   now_us     —— 取微秒函数，NULL 表示不测耗时；
     *   print_task —— 耗时输出，NULL 表示不输出；
     *   kick       —— 按轮喂狗，NULL 表示不喂；
     *   wait       —— 空闲睡眠，NULL 表示不睡；
     *   on_stuck   —— 卡死记录，NULL 表示不记录。
     *
     * 返回：
     *   RES_OK     成功（即使某个任务 init 返回错误，也会继续并返回第一个错误）；
     *   RES_PARAM  对象/表/数组/now_ms 为空等参数错误。   
    */
    result_t (*pf_init)(sched_t *s, const sched_task_t *table, sched_task_rt_t *rt, uint8_t count,
                        sched_now_ms_fn now_ms, sched_now_us_fn now_us, sched_print_fn print_task,
                        sched_kick_fn kick, sched_wait_fn wait, sched_stuck_fn on_stuck);
    /*
     * pf_poll：推进一轮调度。
     * - 毫秒没变：发事件，清 in_poll，调用 wait 睡觉；
     * - 毫秒变了：发事件，跑到期任务，每跑完一个再发事件，最后按轮喂狗。
     * 返回本次遇到的第一个错误（RES_OK 表示都正常）。
     */
    result_t (*pf_poll)(sched_t *s);
    /*
     * pf_on_ms：节拍钩子，由时基中断每毫秒调用一次。
     * 参数：当前毫秒。
     * 行为：如果此刻有任务正在执行，并且已经超过它自己的周期，
     *       调用一次 on_stuck(active=1) 写卡死记录。
     * 不在任务里时直接返回。
     */
    void     (*pf_on_ms)(uint32_t now_ms);
    /*
     * pf_enable：任务开关。
     * 参数：任务编号（任务表下标）、开关（0 停止，非 0 恢复）。
     * 返回：RES_OK / RES_PARAM / RES_STATE。
     */
    result_t (*pf_enable)(uint8_t task_id, uint8_t enable);
} sched_driver_interface_t;

extern const sched_driver_interface_t sched_sw_driver;

#endif /* FW_APP_05_SCHEDULER_SCHED_SCHED_H_ */
