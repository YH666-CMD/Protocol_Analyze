/*
 * config.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 总配置开关（使能模块、功能开关）
 *
 * 调度器不包含本文件。任务周期由 sched_tasks.c 使用。
 * 任务编号必须和 sched_tasks.c 里的表顺序一致。
 */
#ifndef FW_APP_00_SYSTEM_SYSTEM_CONFIG_H_
#define FW_APP_00_SYSTEM_SYSTEM_CONFIG_H_

#define SCHED_TICK_HZ                 1000u     /* 1 ms 一个节拍 */
#define SCHED_TASK_MAX                8u        /* 任务表容量 */
#define SCHED_TIME_MEASURE_ENABLE     1u        /* 1：记录每个任务本次耗时，并在超过周期时报错 */

#define SCHED_ID_LOG                  0u        /* 日志任务 */
#define SCHED_ID_HEARTBEAT            1u        /* 心跳任务 */
#define SCHED_ID_UART                 2u        /* 串口驱动初始化。不读接收环 */
#define SCHED_ID_USB                  3u        /* USB 协议任务。读串口环，组帧上传，拆下行帧 */

#define SCHED_ID_COUNT                4u        /* 任务总数 */

#define SCHED_PERIOD_LOG_MS           100u      /* 日志任务周期 */
#define SCHED_PHASE_LOG_MS            0u        /* 日志任务相位 */
#define SCHED_PERIOD_HEARTBEAT_MS     1000u     /* 心跳任务周期 */
#define SCHED_PHASE_HEARTBEAT_MS      50u       /* 心跳任务相位 */
#define SCHED_PERIOD_UART_MS          5u        /* 串口任务周期。发送已是中断，5ms 够把 115200 的字节搬走，也躲开 1ms 的超周期判断 */
#define SCHED_PHASE_UART_MS           2u        /* 躲开日志相位 0 */
#define SCHED_PERIOD_USB_MS           10u       /* USB 任务周期。一拍可发多帧，115200 下 10ms 环里大约 115 字节 */
#define SCHED_PHASE_USB_MS            7u        /* 躲开日志相位 0 和串口相位 2 */


#define UART_BAUDRATE                 115200u   /*波特率*/
#define UART_RX_TAKE_MAX              128u      /* 上层一次最多从环里拿走的字节数 */


/*
 * 1：测试打印编进程序，只给 USB 日志当出口。
 * 0：格式化不进程序。串口收发和接收环都在。环的消费者是 USB 协议任务，打印不再读环。
 */
#define PRINT_ENABLE                  0u

#define EVT_NONE                      0u        /*默认值0没有其他任务给某个任务传事件*/
#define EVT_LOG_EMERGENCY             1u        /* 事件1：看门狗要求记一条紧急日志 */

#define WDT_ENABLE                    0u        /* 1：使能看门狗，0：不使能看门狗 */
#define WDT_ROUND_MAX_MS              3000u     /* 软件任务一轮最长时间 */
#define WDT_HW_TIMEOUT_MS             4000u     /* 硬件看门狗真实超时时间 */
#define WDT_LOG_WAIT_SPIN             8000u     /* 放弃喂狗前空转这么多次，给紧急 RAM 写完144MHZ大概几微妙 */

#define PORT_TIMEBASE_SPIN            8000000u  /* 等毫秒计数变化的上限 正常1ms就变了，这个已经几百ms了*/

#define LOG_DAILY_DEPTH               8u        /*日常日志在 RAM 里最多攒多少条*/
#define LOG_EMERGENCY_FLAG            0xA5u     /*复位后是否需要写FLASH的标志位，目前在bss段会被清，后续需要加到额外划分ram区间里*/

#if (SCHED_ID_COUNT > SCHED_TASK_MAX)
#error "SCHED_ID_COUNT exceeds SCHED_TASK_MAX"
#endif
#if (SCHED_PHASE_HEARTBEAT_MS >= SCHED_PERIOD_HEARTBEAT_MS) || (SCHED_PHASE_LOG_MS >= SCHED_PERIOD_LOG_MS) || (SCHED_PHASE_UART_MS >= SCHED_PERIOD_UART_MS) || (SCHED_PHASE_USB_MS >= SCHED_PERIOD_USB_MS)
#error "phase must be smaller than period"
#endif
#if (UART_BAUDRATE == 0u) || (UART_RX_TAKE_MAX == 0u)
#error "uart baud and take limit must be non-zero"
#endif

#endif /* FW_APP_00_SYSTEM_SYSTEM_CONFIG_H_ */
