#include "pingpong_buf.h"
#include <string.h>

#define PP_READY_NONE  0u
#define PP_READY_A     1u
#define PP_READY_B     2u

static bool pp_enter(pp_t *pp, uint32_t *state)
{
    if (pp->lock == NULL || pp->unlock == NULL) {
        return false;
    }

    *state = pp->lock();
    return true;
}

static void pp_leave(pp_t *pp, uint32_t state, bool locked)
{
    if (locked) {
        pp->unlock(state);
    }
}

static void pp_note_overrun(pp_t *pp)
{
    if (pp->overrun < UINT32_MAX) {
        pp->overrun++;
    }
}

/* 发布刚填满的半区。上一块还没取走时，DMA 已经回头覆盖它，只保留这一块 */
static void pp_publish(pp_t *pp, uint8_t which)
{
    uint32_t state = 0u;
    bool locked;

    if (pp == NULL) return;

    locked = pp_enter(pp, &state);
    if (pp->ready != PP_READY_NONE) {
        pp_note_overrun(pp);
    }
    pp->ready = which;
    pp_leave(pp, state, locked);
}

static void pp_init(pp_t *pp, pp_lock_fn lock, pp_unlock_fn unlock)
{
    if (pp == NULL) return;

    memset(pp->buf, 0, sizeof(pp->buf));
    pp->ready = PP_READY_NONE;
    pp->overrun = 0u;

    if (lock != NULL && unlock != NULL) {
        pp->lock = lock;
        pp->unlock = unlock;
    } else {
        pp->lock = NULL;
        pp->unlock = NULL;
    }
}

static void pp_irq_half(pp_t *pp)
{
    pp_publish(pp, PP_READY_A);
}

static void pp_irq_full(pp_t *pp)
{
    pp_publish(pp, PP_READY_B);
}

static bool pp_get(pp_t *pp, uint8_t ch_num, pp_sample_t *out, uint16_t *rounds)
{
    uint32_t state = 0u;
    bool locked;
    uint8_t ready;
    const pp_sample_t *src;
    uint16_t per_ch;
    uint16_t n;
    uint8_t ch;

    if (pp == NULL || out == NULL) return false;

    if (rounds != NULL) {
        *rounds = 0u;
    }
    if (ch_num < 1u || ch_num > PP_CH_MAX) return false;
    if ((PP_HALF_SIZE % ch_num) != 0u) return false;

    per_ch = (uint16_t)(PP_HALF_SIZE / ch_num);

    locked = pp_enter(pp, &state);
    ready = pp->ready;
    if (ready == PP_READY_A) {
        src = &pp->buf[0];
    } else if (ready == PP_READY_B) {
        src = &pp->buf[PP_HALF_SIZE];
    } else {
        pp_leave(pp, state, locked);
        return false;
    }

    for (n = 0u; n < per_ch; n++) {
        for (ch = 0u; ch < ch_num; ch++) {
            out[(uint16_t)ch * per_ch + n] = src[n * ch_num + ch];
        }
    }
    pp->ready = PP_READY_NONE;
    pp_leave(pp, state, locked);

    if (rounds != NULL) {
        *rounds = per_ch;
    }
    return true;
}

const pp_driver_interface_t pp_sw_driver =
{
    .pf_pp_init     = pp_init,
    .pf_pp_irq_half = pp_irq_half,
    .pf_pp_irq_full = pp_irq_full,
    .pf_pp_get      = pp_get,
};
