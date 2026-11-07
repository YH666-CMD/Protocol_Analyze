/*
 * bsp_board.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 板级总初始化，统一调用所有BSP初始化
 *
 * bsp_board_init 不启动时基，也不开总中断。
 * 任务对齐完成之后，由 port 调用 bsp_time.pf_start，再调用 bsp_board_irq_on。
 */
#ifndef FW_APP_02_BSP_BOARD_BSP_BOARD_H_
#define FW_APP_02_BSP_BOARD_BSP_BOARD_H_

void bsp_board_init(void);
void bsp_board_irq_on(void);

#endif /* FW_APP_02_BSP_BOARD_BSP_BOARD_H_ */
