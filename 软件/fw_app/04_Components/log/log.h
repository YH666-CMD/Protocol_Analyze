/*
 * log.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 日常日志和紧急日志。FLASH 分区还没划，写入函数先空着。
 *
 * 约定：
 *   1. 日常日志先留在 RAM。日志任务周期到了再尝试写入 FLASH。
 *   2. 紧急日志由看门狗钩子或节拍中断直接写入 g_log_emergency，不依赖调度器还活着。
 *   3. g_log_emergency 第一个字节是标志。上电后第一件事看这个标志，要写再进 FLASH。
 *   4. 这块 RAM 以后要放进复位不清零的段。现在还在普通 bss 里，复位后标志会丢。
 *   5. 分区没划出来之前，空的 FLASH 函数返回 RES_STATE，不清 RAM，也不当成故障。
 */
#ifndef FW_APP_04_COMPONENTS_LOG_LOG_H_
#define FW_APP_04_COMPONENTS_LOG_LOG_H_

#include <stdint.h>
#include "platform.h"

typedef struct
{
    uint8_t  flag;   /* LOG_EMERGENCY_FLAG：里面有还没进 FLASH 的紧急记录 */
    uint8_t  module; /* 任务没返回时是任务编号。看门狗那条写 0 */
    uint16_t arg;    /* 任务没返回时是已经过去的毫秒，超过 65535 就记满 */
    uint32_t code;   /* 故障码。任务没返回时是 PLAT_ERR_SCHED_STUCK */
    uint32_t extra;  /* 看门狗传入的附加参数，或卡住时的完整毫秒数 */
} log_emergency_t;

/*全局唯一的一条紧急记录。链接脚本划出 NOINIT 之前不要把它初始化清掉。 */
extern volatile log_emergency_t g_log_emergency;

typedef struct
{
    result_t (*pf_boot)(void);  /*上电后、别的业务之前调用。标志有效就尝试把紧急 RAM 写入 FLASH */
    result_t (*pf_init)(void);  /*初始化日常日志。只清日常记录，不动紧急 RAM */
    /*
     * pf_daily：追加一条日常记录。
     *
     * 参数：
     *   module —— 模块号（谁记的）；
     *   event  —— 事件号（发生了什么）；
     *   arg    —— 一个 16 位附加参数（例如长度、错误码、计数）。
     *
     * 返回：
     *   RES_OK     追加成功；
     *   RES_NO_MEM 数组已满，新记录被丢弃（旧记录保留）。
    */
    result_t (*pf_daily)(uint8_t module, uint8_t event, uint16_t arg);
    /*
     * pf_emergency：写一条紧急记录（只写 RAM）。
     *
     * 参数：
     *   code  —— 故障码（通常是 plat_err_t）；
     *   extra —— 附加参数（看门狗这里传“这一轮用了多少毫秒”）。
     *
     * 行为：
     *   - 无条件覆盖现有紧急记录；
     *   - module/arg 清零；
     *   - 最后写 flag = 0xA5。
    */
    result_t (*pf_emergency)(uint32_t code, uint32_t extra);
    /*
     * pf_stuck：记录“某个任务超过自己的周期还没返回”。
     *
     * 参数：
     *   task_id    —— 任务编号；
     *   elapsed_ms —— 已经卡了多少毫秒。
     *
     * 行为：
     *   - 如果已经有一条“非卡住”的紧急记录，返回 RES_BUSY，不覆盖；
     *   - 否则写 module=task_id、arg=min(elapsed,65535)、extra=完整毫秒、
     *     code=PLAT_ERR_SCHED_STUCK、flag=0xA5。
    */
    result_t (*pf_stuck)(uint8_t task_id, uint32_t elapsed_ms);
    result_t (*pf_stuck_clear)(void);    /*任务后来返回了，撤掉“卡住”这条记录。*/
    result_t (*pf_flush_daily)(void);    /*尝试把日常 RAM 记录写进 Flash*/
    result_t (*pf_flush_emergency)(void);/*尝试把紧急 RAM 记录写进 Flash*/
} log_driver_t;

extern const log_driver_t log_sw_driver;

#endif /* FW_APP_04_COMPONENTS_LOG_LOG_H_ */
