#include "port.h"
#include "bsp_board.h"
#include "bsp_time.h"
#include "sched.h"
#include "sched_tasks.h"
#include <stddef.h>

void app_init(void)
{
    const sched_task_t *table = NULL;
    sched_task_rt_t *rt = NULL;
    uint8_t count = 0u;

    bsp_board_init();/*板级初始化*/
    sched_tasks_bind(&table, &rt, &count);/*拿到整机任务表*/
    sched_sw_driver.pf_init(table, rt, count, bsp_time.pf_now_ms, NULL, NULL);/*初始化调度器*/
    bsp_time.pf_start();/*启动1ms时基*/
    bsp_board_irq_on();/*开总中断*/
}

void app_loop(void)
{
    sched_sw_driver.pf_poll();
}
