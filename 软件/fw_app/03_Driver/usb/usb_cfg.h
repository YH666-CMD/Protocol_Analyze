/*
 * usb_cfg.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 移植时要改的开关。只放宏，不放函数。
 *
 * 换板子时先改本文件，再核对 usb_conf.h 里的缓冲区地址。
 * 本文件不包含芯片头，别的工程也能先读这些开关。
 *
 * 约定：
 *   1. USB_HCLK_HZ 必须和 SystemInit 里的主频一致。全速 USB 内核要 48 MHz，
 *      驱动按这个宏选分频，不再在运行时读时钟树。
 *   2. USB_VID、USB_PID 会写进设备描述符。默认是沁恒虚拟串口 1A86:FE0C。
 *   3. USB_EP0_SIZE、USB_EP_MPS 改了以后，usb_conf.h 里各端点缓冲区的间隔也要改。
 *      现在按 64 字节排布，间隔是 0x40。
 *   4. USB_LOG_ENABLE 为 0 时，打印函数和格式串都不进程序。
 */
#ifndef FW_APP_03_DRIVER_USB_USB_CFG_H_
#define FW_APP_03_DRIVER_USB_USB_CFG_H_

/* 主频。只允许 48 / 96 / 144 MHz，对应 USB 时钟 1 / 2 / 3 分频。 */
#ifndef USB_HCLK_HZ
#define USB_HCLK_HZ                 96000000u
#endif

/* 设备描述符里的厂商和产品。小端写入。 */
#ifndef USB_VID
#define USB_VID                     0x1A86u
#endif
#ifndef USB_PID
#define USB_PID                     0xFE0Cu
#endif

/* 端点 0 最大包长，也是描述符里的 bMaxPacketSize0。全速只能是 8、16、32、64。 */
#ifndef USB_EP0_SIZE
#define USB_EP0_SIZE                64u
#endif

/* 批量端点和中断端点的最大包长。本描述符按 64 字节填写。 */
#ifndef USB_EP_MPS
#define USB_EP_MPS                  64u
#endif

/* 拉上拉之前先断开这么久，主机才会当成一次重新插入。单位毫秒。 */
#ifndef USB_DISC_MS
#define USB_DISC_MS                 20u
#endif

/* 1：用片内 1.5k 上拉。0：上拉由板子外部提供，软件不再打开 EXTEN。 */
#ifndef USB_USE_INTERNAL_PU
#define USB_USE_INTERNAL_PU         1u
#endif

/* USB 低优先级中断。抢占级要低于系统滴答，避免枚举被时基打断之外的事。 */
#ifndef USB_LP_PREEMPT
#define USB_LP_PREEMPT              1u
#endif
#ifndef USB_LP_SUB
#define USB_LP_SUB                  0u
#endif

/* 唤醒中断。挂起之后靠它回来，抢占级高于 USB 低优先级。 */
#ifndef USB_WKUP_PREEMPT
#define USB_WKUP_PREEMPT            0u
#endif
#ifndef USB_WKUP_SUB
#define USB_WKUP_SUB                0u
#endif

/*
 * PA11 是 D-，PA12 是 D+。下面三个值是 GPIOA->CFGHR 里这两个脚的写法。
 * 换引脚时只改这三个数，并确认仍落在 CFGHR（引脚 8 到 15）。
 * CLR 清掉原模式，IN 是浮空输入，OUT 是推挽输出，用来假装拔线。
 */
#ifndef USB_GPIO_CFGHR_CLR
#define USB_GPIO_CFGHR_CLR          0xFFF00FFFu
#endif
#ifndef USB_GPIO_CFGHR_IN
#define USB_GPIO_CFGHR_IN           0x00044000u
#endif
#ifndef USB_GPIO_CFGHR_OUT
#define USB_GPIO_CFGHR_OUT          0x00033000u
#endif

/* 1：编译打印函数。0：调用点被消掉。默认关，避免占代码空间。 */
#ifndef USB_LOG_ENABLE
#define USB_LOG_ENABLE              0u
#endif

#if (USB_HCLK_HZ != 48000000u) && (USB_HCLK_HZ != 96000000u) && (USB_HCLK_HZ != 144000000u)
#error "USB_HCLK_HZ must be 48000000, 96000000 or 144000000"
#endif
#if (USB_EP0_SIZE != 8u) && (USB_EP0_SIZE != 16u) && (USB_EP0_SIZE != 32u) && (USB_EP0_SIZE != 64u)
#error "USB_EP0_SIZE must be 8, 16, 32 or 64"
#endif
#if (USB_EP_MPS != 8u) && (USB_EP_MPS != 16u) && (USB_EP_MPS != 32u) && (USB_EP_MPS != 64u)
#error "USB_EP_MPS must be 8, 16, 32 or 64"
#endif
#if (USB_DISC_MS == 0u)
#error "USB_DISC_MS must be non-zero"
#endif

#endif /* FW_APP_03_DRIVER_USB_USB_CFG_H_ */
