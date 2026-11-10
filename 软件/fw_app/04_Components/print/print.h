/*
 * print.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 测试阶段的打印。格式化在这里，字节从注入的函数送出去。
 *
 * 约定：
 *   1. PRINT_ENABLE 为 0 时函数仍可调用，但一个字节都不发送。
 *   2. 本文件不出现串口寄存器。发送一个字节的函数由调用方传进来。
 *   3. pf_printf 支持 %c %s %d %u %x %X %%，以及 %02x 这种宽度和补零。没有浮点。
 *   4. pf_write 原样发送一段字节，不追加换行。消费者用它把环里的数据打出去。
 */
#ifndef FW_APP_04_COMPONENTS_PRINT_PRINT_H_
#define FW_APP_04_COMPONENTS_PRINT_PRINT_H_

#include <stdint.h>
#include "platform.h"

typedef result_t (*print_putc_fn)(uint8_t ch);

typedef struct
{
    /*
     * pf_init：记下发送一个字节的函数。
     * 形参：putc —— 不能为 NULL（PRINT_ENABLE 为 0 时忽略）。
     * 返回：RES_OK / RES_PARAM。
     */
    result_t (*pf_init)(print_putc_fn putc);
    /*
     * pf_write：原样发送。
     * 形参：data —— 字节地址；len —— 字节数。
     * 返回：RES_OK；RES_PARAM 数据指针为空但长度不是 0；RES_STATE 还没初始化；
     *       以及 putc 返回的错误。中途失败时，前面的字节已经发出去了。
     */
    result_t (*pf_write)(const uint8_t *data, uint16_t len);
    /*
     * pf_printf：按格式发送。
     * 形参：fmt —— 格式串，不能为 NULL。
     * 返回和 pf_write 相同。
     */
    result_t (*pf_printf)(const char *fmt, ...);
} print_driver_t;

extern const print_driver_t print_sw_driver;

#endif /* FW_APP_04_COMPONENTS_PRINT_PRINT_H_ */
