/*
 * platform.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 返回码和异常钩子。定位问题时看钩子名字和 g_plat_last_fault。
 *
 * 约定：
 *   1. 每种异常一个函数，函数名就是这种异常。检测到问题就 plat_raise 对应钩子。
 *   2. 钩子里可以打印、修复或记录。返回值留给调用方接着判断。
 *   3. fatal 为 1 的钩子返回后停住。看门狗复位不走这里。
 *   4. 新异常往后加枚举和钩子，不复用已经用过的数值。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_PLATFORM_H_
#define FW_APP_00_SYSTEM_SYSTEM_PLATFORM_H_

#include <stdint.h>

typedef enum
{
    RES_OK = 0,    /* 成功 */
    RES_PARAM,     /* 参数非法 */
    RES_BUSY,      /* 现在不能做 */
    RES_NO_MEM,    /* 队列或缓冲区满 */
    RES_STATE,     /* 状态不对，或这块功能还没接上 */
    RES_OVERRUN    /* 这次调用花的时间超过了任务周期 */
} result_t;/*普通函数的返回值*/


typedef enum
{
    PLAT_OK = 0,
    PLAT_ERR_SCHED_BAD_TABLE = 1,  /* 任务表不合法 */
    PLAT_ERR_SCHED_REENTER   = 2,  /* 任务里又进了一次调度 */
    PLAT_ERR_SCHED_OVERRUN   = 3,  /* 任务耗时超过自己的周期 */
    PLAT_ERR_TIMEBASE        = 4,  /* 毫秒时基没有走 */
    PLAT_ERR_WDT_HANG        = 5,  /* 一轮任务超时，看门狗准备放弃喂狗 */
    PLAT_ERR_EVT_FULL        = 6,  /* 目标任务邮箱里还有没处理的事件 */
    PLAT_ERR_LOG_FLASH       = 10, /* 日志写入 FLASH 失败。7 到 9 空着，不要复用 */
    PLAT_ERR_TASK            = 11, /* 任务函数返回了错误 */
    PLAT_ERR_SCHED_STUCK     = 12  /* 任务超过自己的周期还没返回。节拍中断写进紧急日志，不走 plat_raise */
} plat_err_t;/*异常/故障码*/


typedef enum
{
    PLAT_RESET_NONE = 0,     /* 还没读过 */
    PLAT_RESET_PIN,          /* 引脚复位：NRST 被外部拉低 */
    PLAT_RESET_POR,          /* 上电或掉电复位 */
    PLAT_RESET_SOFTWARE,     /* 软件复位 */
    PLAT_RESET_IWDG,         /* 独立看门狗复位 */
    PLAT_RESET_WWDG,         /* 窗口看门狗复位 */
    PLAT_RESET_LOWPOWER,     /* 低功耗复位 */
    PLAT_RESET_UNKNOWN       /* 没有认出的复位标志 */
} plat_reset_t;/*复位原因*/
/*异常钩子的类型,不同异常含义不同*/
typedef int32_t (*plat_hook_fn)(uint32_t arg0, uint32_t arg1);

typedef struct
{
    const char  *name;  /* 调试器里直接看的异常名字 */
    plat_hook_fn fn;    /* 异常处理函数，不能为 NULL（plat_raise 会检查） */
    uint8_t      fatal; /* 1：回调返回后停住；0：返回给调用者继续判断 */
} plat_hook_t;

typedef struct
{
    const char *name;  /*钩子名字的指针（指向 plat_hook_t.name 的静态字符串）*/
    uint32_t    arg0;  /*第一个参数*/
    uint32_t    arg1;  /*第二个参数*/
    int32_t     result;/*钩子函数返回的值*/
} plat_fault_info_t;


extern volatile plat_fault_info_t g_plat_last_fault;/*异常信息*/


extern volatile uint32_t g_plat_reset_reason;/*复位原因*/


int32_t plat_raise(const plat_hook_t *hook, uint32_t arg0, uint32_t arg1);/*异常钩子的统一入口*/


int32_t plat_on_reset(uint32_t reason);/*获取复位原因*/


void plat_on_task_stuck(uint8_t task_id, uint32_t elapsed_ms, uint8_t active);/*任务超过周期还没返回,(任务编号，卡了多少毫秒，1还没返回写下紧急记录0任务随后返回了撤销这条卡住记录)*/

extern const plat_hook_t plat_hook_sched_bad_table;
extern const plat_hook_t plat_hook_sched_reenter;
extern const plat_hook_t plat_hook_sched_overrun;
extern const plat_hook_t plat_hook_timebase_dead;
extern const plat_hook_t plat_hook_wdt_hang;
extern const plat_hook_t plat_hook_evt_full;
extern const plat_hook_t plat_hook_log_flash;
extern const plat_hook_t plat_hook_task_fail;

#endif /* FW_APP_00_SYSTEM_SYSTEM_PLATFORM_H_ */
