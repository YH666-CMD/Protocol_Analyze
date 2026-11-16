/*
 * pingpong_buf.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 通用乒乓缓冲区（DMA 半传输 / 全传输）。业务层只依赖本契约。
 *
 * 约定：
 *   1. 一个外设一个 pp_t。DMA 中断里发布半区，主循环里取走。
 *   2. 缓冲区不绑定通道数。1 到 5 路都在取数据时用 ch_num 区分。
 *      半区样本数是 1、2、3、4、5 的公倍数，所以每一路都从通道 0 开始。
 *   3. 连续 DMA 同时只有一个半区是稳定的。上一个半区没取走又来了新半区，计一次 overrun，只保留最新半区。
 *   4. pf_pp_get 按 ch_num 把半区拆开后拷出。同一轮的顺序是通道 0 到通道 ch_num-1。
 *      输出按通道排开：out[通道号 * 轮数 + 轮号]。调用方数组至少要有 PP_HALF_SIZE 个样本。
 *   5. 下一次半区完成前必须取走。关中断挡不住 DMA 写内存。
 *   6. 启动 DMA 之前调用 pf_pp_init。lock/unlock 要挡住这条 DMA 中断，两个都传，或都传 NULL。
 */
#ifndef FW_APP_04_COMPONENTS_PINGPONG_BUF_PINGPONG_BUF_H_
#define FW_APP_04_COMPONENTS_PINGPONG_BUF_PINGPONG_BUF_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t pp_sample_t;

/* 关中断时返回原来的中断状态，开中断时原样传回。两个都传，或都传 NULL。 */
typedef uint32_t (*pp_lock_fn)(void);
typedef void     (*pp_unlock_fn)(uint32_t state);

#define PP_CH_MAX      5u
/*
 * 半区样本数必须同时是 1、2、3、4、5 的倍数，最小公倍数是 60。
 * 每个样本 2 字节，两个半区一共 240 字节。256 不能满足全部通道数。
 * DMA 用循环模式、半字宽度，个数填 PP_BUF_SIZE（120 个半字）。
 */
#define PP_HALF_SIZE   120u
#define PP_BUF_SIZE    (2u * PP_HALF_SIZE)
#define PP_BUF_BYTES   (PP_BUF_SIZE * 2u)

#if (PP_HALF_SIZE == 0u) || ((PP_HALF_SIZE % 60u) != 0u)
#error "PP_HALF_SIZE must be a positive multiple of 60 so 1..5 channels all start at channel 0"
#endif

/* 缓冲区对象。存储放在调用方，接口本身不带芯片信息 */
typedef struct
{
    pp_sample_t       buf[PP_BUF_SIZE];
    volatile uint8_t  ready;    /* 0 无数据，1 为 A 区，2 为 B 区。由接口维护 */
    volatile uint32_t overrun;  /* 半区没取走就被新数据覆盖的次数 */
    pp_lock_fn        lock;     /* 由 pf_pp_init 写入 */
    pp_unlock_fn      unlock;
} pp_t;

/*
 * 乒乓缓冲工具箱。函数指针由具体实现填入。
 * 半传输中断调用 pf_pp_irq_half，全传输中断调用 pf_pp_irq_full。
 */
typedef struct
{
    void (*pf_pp_init)(pp_t *pp, pp_lock_fn lock, pp_unlock_fn unlock);
    void (*pf_pp_irq_half)(pp_t *pp);
    void (*pf_pp_irq_full)(pp_t *pp);
    /*
     * ch_num 取 1 到 5。out 按通道排开，至少 PP_HALF_SIZE 个样本。
     * *rounds 是每个通道的样本数。通道数不合法时不取走半区。
     */
    bool (*pf_pp_get)(pp_t *pp, uint8_t ch_num, pp_sample_t *out, uint16_t *rounds);
} pp_driver_interface_t;

/* 纯软件实现，结构体放在 Flash */
extern const pp_driver_interface_t pp_sw_driver;

#endif /* FW_APP_04_COMPONENTS_PINGPONG_BUF_PINGPONG_BUF_H_ */
