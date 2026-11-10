/*
 * ring_buffer.h
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: 通用循环缓冲区（串口、USB 复用）。业务层只依赖本契约。
 *
 * 约定：
 *   1. 单生产者、单消费者。串口中断里只写，主循环里只读。
 *   2. 空出一个格子区分满和空，最多存放 RB_SIZE-1 个字节。
 *   3. 满了就丢弃新字节，不覆盖旧数据。overwrite_count 统计被丢弃的字节数。
 *   4. 使用前调用接口里的 pf_rb_init。
 */
#ifndef FW_APP_04_COMPONENTS_RING_BUFFER_RING_BUFFER_H_
#define FW_APP_04_COMPONENTS_RING_BUFFER_RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * 容量必须是 2 的幂，这样下标回绕可以用按位与。
 * 接收是逐字节进环，任务每 5ms 搬走。256 在 115200 下大约能攒 22ms。
 */
#define RB_SIZE   256

#if (RB_SIZE < 2) || (RB_SIZE > 65536) || ((RB_SIZE & (RB_SIZE - 1)) != 0)
#error "RB_SIZE must be a power of two in the range 2..65536"
#endif

/* 缓冲区对象。存储放在调用方，接口本身不带芯片信息 */
typedef struct
{
    uint8_t           buf[RB_SIZE];
    volatile uint16_t head;            /* 下一个要写的位置，只由生产者修改 */
    volatile uint16_t tail;            /* 下一个要读的位置，只由消费者修改 */
    volatile uint32_t overwrite_count; /* 因缓冲区满而丢弃的字节数 */
} rb_t;

/*
 * 循环缓冲工具箱。函数指针由具体实现填入。
 * 帧同步的格式：第 1 字节是同步字，第 2 字节是整帧长度（含这两字节）。
 * 长度不合法的假帧头会被跳过；长度合法但数据没到齐时停在帧头。
 * min_len 小于 2 时按 2 处理。半包超时后用 pf_rb_read 丢掉 1 字节再同步。
 */
typedef struct
{
    void     (*pf_rb_init)(rb_t *rb);
    void     (*pf_rb_clear)(rb_t *rb);
    uint16_t (*pf_rb_available)(rb_t *rb);
    bool     (*pf_rb_is_empty)(rb_t *rb);
    bool     (*pf_rb_is_full)(rb_t *rb);
    bool     (*pf_rb_write)(rb_t *rb, uint8_t data);
    uint16_t (*pf_rb_write_buf)(rb_t *rb, const uint8_t *src, uint16_t len);
    bool     (*pf_rb_read)(rb_t *rb, uint8_t *out);
    uint16_t (*pf_rb_read_buf)(rb_t *rb, uint8_t *out, uint16_t len);
    bool     (*pf_rb_peek_line)(rb_t *rb, uint8_t end_char, uint16_t *frame_len);
    bool     (*pf_rb_sync_and_peek)(rb_t *rb, uint8_t sync_byte, uint16_t min_len, uint16_t max_len, uint16_t *frame_len);
} rb_driver_interface_t;

/* 纯软件实现，结构体放在 Flash */
extern const rb_driver_interface_t rb_sw_driver;

#endif /* FW_APP_04_COMPONENTS_RING_BUFFER_RING_BUFFER_H_ */
