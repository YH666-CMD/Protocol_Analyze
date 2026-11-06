#include "ring_buffer.h"
#include <stddef.h>

#define RB_MASK   (RB_SIZE - 1)

/* 保证数据先写进 buf，再发布 head/tail。中断和主循环之间不能重排这两步 */
static void rb_barrier(void)
{
    __asm__ volatile("" ::: "memory");
}

static void rb_note_drop(rb_t *rb, uint32_t n)
{
    uint32_t count = rb->overwrite_count;

    if (n >= (UINT32_MAX - count)) {
        rb->overwrite_count = UINT32_MAX;
    } else {
        rb->overwrite_count = count + n;
    }
}

static uint16_t rb_available(rb_t *rb)
{
    uint16_t head;
    uint16_t tail;

    if (rb == NULL) return 0;

    head = (uint16_t)(rb->head & RB_MASK);
    tail = (uint16_t)(rb->tail & RB_MASK);
    return (uint16_t)((head - tail) & RB_MASK);
}

static void rb_init(rb_t *rb)
{
    if (rb == NULL) return;

    rb->head = 0;
    rb->tail = 0;
    rb->overwrite_count = 0;
}

static void rb_clear(rb_t *rb)
{
    uint16_t head;

    if (rb == NULL) return;

    head = (uint16_t)(rb->head & RB_MASK);
    rb_barrier();
    rb->tail = head;
}

static bool rb_is_empty(rb_t *rb)
{
    if (rb == NULL) return true;

    return ((rb->head & RB_MASK) == (rb->tail & RB_MASK));
}

static bool rb_is_full(rb_t *rb)
{
    uint16_t tail;
    uint16_t head;

    if (rb == NULL) return true;

    tail = (uint16_t)(rb->tail & RB_MASK);
    head = (uint16_t)(rb->head & RB_MASK);
    return ((uint16_t)((head + 1u) & RB_MASK) == tail);
}

static bool rb_write(rb_t *rb, uint8_t data)
{
    uint16_t tail;
    uint16_t head;
    uint16_t next_head;

    if (rb == NULL) return false;

    tail = (uint16_t)(rb->tail & RB_MASK);
    head = (uint16_t)(rb->head & RB_MASK);
    next_head = (uint16_t)((head + 1u) & RB_MASK);
    if (next_head == tail) {
        rb_note_drop(rb, 1u);
        return false;
    }

    rb->buf[head] = data;
    rb_barrier();
    rb->head = next_head;
    return true;
}

static uint16_t rb_write_buf(rb_t *rb, const uint8_t *src, uint16_t len)
{
    uint16_t written = 0;

    if (rb == NULL || src == NULL) return 0;

    while (written < len) {
        uint16_t tail = (uint16_t)(rb->tail & RB_MASK);
        uint16_t head = (uint16_t)(rb->head & RB_MASK);
        uint16_t next_head = (uint16_t)((head + 1u) & RB_MASK);

        if (next_head == tail) {
            rb_note_drop(rb, (uint32_t)(len - written));
            break;
        }

        rb->buf[head] = src[written++];
        rb_barrier();
        rb->head = next_head;
    }
    return written;
}

static bool rb_read(rb_t *rb, uint8_t *out)
{
    uint16_t head;
    uint16_t tail;

    if (rb == NULL || out == NULL) return false;

    head = (uint16_t)(rb->head & RB_MASK);
    tail = (uint16_t)(rb->tail & RB_MASK);
    if (head == tail) return false;

    rb_barrier();
    *out = rb->buf[tail];
    rb_barrier();
    rb->tail = (uint16_t)((tail + 1u) & RB_MASK);
    return true;
}

static uint16_t rb_read_buf(rb_t *rb, uint8_t *out, uint16_t len)
{
    uint16_t n = 0;

    if (rb == NULL || out == NULL) return 0;

    while (n < len && rb_read(rb, &out[n])) {
        n++;
    }
    return n;
}

static bool rb_peek_line(rb_t *rb, uint8_t end_char, uint16_t *frame_len)
{
    uint16_t tail;
    uint16_t head;
    uint16_t n = 0;

    if (rb == NULL || frame_len == NULL) return false;

    *frame_len = 0;
    tail = (uint16_t)(rb->tail & RB_MASK);
    head = (uint16_t)(rb->head & RB_MASK);
    rb_barrier();

    while (tail != head && n < (uint16_t)(RB_SIZE - 1u)) {
        n++;
        if (rb->buf[tail] == end_char) {
            *frame_len = n;
            return true;
        }
        tail = (uint16_t)((tail + 1u) & RB_MASK);
    }
    return false;
}

static bool rb_sync_and_peek(rb_t *rb, uint8_t sync_byte, uint16_t min_len, uint16_t max_len, uint16_t *frame_len)
{
    uint16_t guard = RB_SIZE;
    uint16_t need_min = min_len;

    if (rb == NULL || frame_len == NULL) return false;

    *frame_len = 0;
    if (need_min < 2u) need_min = 2u;
    if (max_len > (uint16_t)(RB_SIZE - 1u)) max_len = (uint16_t)(RB_SIZE - 1u);
    if (need_min > max_len) return false;

    while (guard-- > 0u) {
        uint16_t tail = (uint16_t)(rb->tail & RB_MASK);
        uint16_t head = (uint16_t)(rb->head & RB_MASK);
        uint16_t avail = (uint16_t)((head - tail) & RB_MASK);
        uint16_t t = tail;
        uint16_t a = avail;
        uint8_t pack_len;

        if (avail == 0u) return false;

        rb_barrier();
        while (a > 0u && rb->buf[t] != sync_byte) {
            t = (uint16_t)((t + 1u) & RB_MASK);
            a--;
        }
        if (a == 0u) {
            /* 只丢掉已经看过的这段，不用最新的 head，避免中断刚写入的字节被清掉 */
            rb_barrier();
            rb->tail = head;
            return false;
        }

        rb_barrier();
        rb->tail = t;

        if (a < 2u) return false;

        pack_len = rb->buf[(t + 1u) & RB_MASK];
        if (pack_len < need_min || pack_len > max_len) {
            rb_barrier();
            rb->tail = (uint16_t)((t + 1u) & RB_MASK);
            continue;
        }
        if (a < pack_len) return false;

        *frame_len = pack_len;
        return true;
    }

    return false;
}

const rb_driver_interface_t rb_sw_driver =
{
    .pf_rb_init          = rb_init,
    .pf_rb_clear         = rb_clear,
    .pf_rb_available     = rb_available,
    .pf_rb_is_empty      = rb_is_empty,
    .pf_rb_is_full       = rb_is_full,
    .pf_rb_write         = rb_write,
    .pf_rb_write_buf     = rb_write_buf,
    .pf_rb_read          = rb_read,
    .pf_rb_read_buf      = rb_read_buf,
    .pf_rb_peek_line     = rb_peek_line,
    .pf_rb_sync_and_peek = rb_sync_and_peek,
};
