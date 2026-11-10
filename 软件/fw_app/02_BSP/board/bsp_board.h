/*
 * bsp_board.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 板级总初始化，统一调用所有BSP初始化
 *
 * bsp_board_init 不启动时基，也不开总中断。
 * 它的第一件事是读复位原因并交给 plat_on_reset，然后清掉芯片里的复位标志。
 * 任务对齐完成之后，由 port 调用 bsp_time.pf_start，再调用 bsp_board_irq_on。
 */
#ifndef FW_APP_02_BSP_BOARD_BSP_BOARD_H_
#define FW_APP_02_BSP_BOARD_BSP_BOARD_H_

#include <stdint.h>

void bsp_board_init(void);
void bsp_board_irq_on(void);

/* 返回关中断前的状态，unlock 原样传回。给事件队列注入 */
uint32_t bsp_board_irq_lock(void);
void bsp_board_irq_unlock(uint32_t state);

#endif /* FW_APP_02_BSP_BOARD_BSP_BOARD_H_ */
