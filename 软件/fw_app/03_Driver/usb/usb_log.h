/*
 * usb_log.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 调试打印。格式在本模块，字节由外部函数送出。
 *
 * 约定：
 *   1. USB_LOG_ENABLE 为 0 时，本文件里的调用都变成空宏，不产生指令，也不引用外部打印。
 *   2. 为 1 时，先 usb_log_bind 登记一个 printf 风格的函数，再产生日志。
 *      没登记就什么都不发。
 *   3. 登记的函数会在 USB 中断里被调用。它不能睡眠，不能等 USB，不能长时间关中断。
 *      本工程传 print_sw_driver.pf_printf，那个函数只把字节放进串口队列。
 *   4. 不打印 SOF。SOF 每毫秒一次，打开就会把队列打满。
 *   5. 配置前的挂起只打第一次。次数攒到下一次总线复位再打一行，避免把队列打满。
 */
#ifndef FW_APP_03_DRIVER_USB_USB_LOG_H_
#define FW_APP_03_DRIVER_USB_USB_LOG_H_

#include <stdint.h>
#include "platform.h"
#include "usb_cfg.h"

/* 和 printf 一样的可变参数。返回值沿用工程里的 result_t，本模块不解释它。 */
typedef result_t (*usb_log_fn)(const char *fmt, ...);

#if USB_LOG_ENABLE

/*
 * usb_log_bind：登记输出函数。
 * 形参：fn —— printf 风格。NULL 表示先关掉输出。
 * 在 pf_init 之前调用。重复调用以最后一次为准。
 */
void usb_log_bind(usb_log_fn fn);

/* 上拉已经打开，主机应当开始复位。 */
void usb_log_attach(void);

/*
 * usb_log_reset：总线复位已经处理完。
 * 一行里带三个寄存器，都是处理后的值：
 *   cntr  控制寄存器。正常是 bf00。多出的 bit4 表示恢复信号还占着总线。
 *   ep0   端点 0。复位后应是 3210：接收有效，发送停顿。
 *   fnr   帧号寄存器。bit15 是 D+，bit14 是 D-，bit13 是帧已锁定。
 */
void usb_log_reset(void);

/*
 * usb_log_setup：端点 0 收到一次主机请求。
 * 形参按 SETUP 包的字段：类型、请求号、wValue、wLength。
 * 在中断里、原厂处理返回之后调用，此时请求已经被解析。
 * 只在端点的 SETUP 位置位时打。同一次控制传输末尾的状态包不再打，避免同一行出现两次。
 */
void usb_log_setup(uint8_t type, uint8_t req, uint16_t value, uint16_t length);

/* 主机 SET_ADDRESS 已经写进设备。addr 是新地址。 */
void usb_log_address(uint8_t addr);

/* 主机选定了配置。cfg 非 0 表示枚举完成，设备应进入已配置。 */
void usb_log_configured(uint8_t cfg);

/*
 * usb_log_pma_ovr：分组缓冲溢出。CPU 没能及时应答这次缓冲访问，这一包不可信。
 * 一行里带：
 *   rc  端点 0 接收计数。高 6 位是缓冲区大小，8408 表示 64 字节配置还在且收到 8 字节。
 *       0008 表示高 6 位没了，数据多半没写进缓冲。
 *   d   接收缓冲区第一个半字。0680 是 GET_DESCRIPTOR。和标记值相同说明硬件没写这格。
 *   ep  当时的端点 0 寄存器。
 */
void usb_log_pma_ovr(void);

/* 传输错误。istr 是出错那次的中断状态，bit13 是错误标志。 */
void usb_log_err(uint16_t istr);

/* 配置完成前收到挂起。已清掉恢复位，避免自己把总线驱动住。 */
void usb_log_resume_blocked(void);

/* 丢失 SOF 后等复位等了太久，没有复位整条总线。 */
void usb_log_esof_bail(void);

/*
 * usb_log_line：主机写下 CDC 线路参数。
 * 形参：raw —— 7 字节，布局和 CDC 规范一致：波特率小端 4 字节、停止位、校验、数据位。
 *        不能为 NULL。
 */
void usb_log_line(const uint8_t raw[7]);

/* 端点 2 收到一包。len 是字节数。数据内容不打印。 */
void usb_log_rx(uint16_t len);

/*
 * usb_log_tx：端点 3 提交一包的结果。
 * 形参：len —— 想发的字节数；ok —— 1 已经交给端点，0 上一包还没发完或参数不对。
 */
void usb_log_tx(uint16_t len, uint8_t ok);

#else

#define usb_log_bind(fn)                         ((void)0)
#define usb_log_attach()                         ((void)0)
#define usb_log_reset()                          ((void)0)
#define usb_log_setup(type, req, value, length)  ((void)0)
#define usb_log_address(addr)                    ((void)0)
#define usb_log_configured(cfg)                  ((void)0)
#define usb_log_pma_ovr()                        ((void)0)
#define usb_log_err(istr)                        ((void)0)
#define usb_log_resume_blocked()                 ((void)0)
#define usb_log_esof_bail()                      ((void)0)
#define usb_log_line(raw)                        ((void)0)
#define usb_log_rx(len)                          ((void)0)
#define usb_log_tx(len, ok)                      ((void)0)

#endif

#endif /* FW_APP_03_DRIVER_USB_USB_LOG_H_ */
