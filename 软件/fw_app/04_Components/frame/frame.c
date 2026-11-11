#include "frame.h"
#include "crc.h"
#include <stddef.h>

static result_t frame_pack(uint8_t cmd, const uint8_t *data, uint16_t data_len,
                           uint8_t *out, uint16_t out_cap, uint16_t *out_len)
{
    uint16_t need;
    uint16_t crc;
    uint16_t i;

    if (out == NULL || out_len == NULL)
    {
        return RES_PARAM;
    }
    if (data_len != 0u && data == NULL)
    {
        return RES_PARAM;
    }
    if (data_len > FRAME_DATA_MAX)
    {
        return RES_PARAM;
    }

    need = (uint16_t)(FRAME_OH + data_len);
    if (out_cap < need)
    {
        return RES_PARAM;
    }

    out[0] = FRAME_SOF;
    out[1] = (uint8_t)need;
    out[2] = cmd;
    for (i = 0u; i < data_len; i++)
    {
        out[3u + i] = data[i];
    }

    crc = crc_sw_driver.pf_crc16_ccitt(out, (uint32_t)(need - 2u));
    out[need - 2u] = (uint8_t)(crc & 0xFFu);
    out[need - 1u] = (uint8_t)(crc >> 8);
    *out_len = need;
    return RES_OK;
}

static result_t frame_unpack(const uint8_t *in, uint16_t in_len,
                             uint8_t *cmd, const uint8_t **data, uint16_t *data_len)
{
    uint16_t crc;
    uint16_t got;
    uint16_t payload;

    if (in == NULL || cmd == NULL || data == NULL || data_len == NULL)
    {
        return RES_PARAM;
    }
    if (in_len < FRAME_MIN || in_len > FRAME_MAX)
    {
        return RES_PARAM;
    }
    if (in[0] != FRAME_SOF || (uint16_t)in[1] != in_len)
    {
        return RES_PARAM;
    }

    crc = crc_sw_driver.pf_crc16_ccitt(in, (uint32_t)(in_len - 2u));
    got = (uint16_t)in[in_len - 2u] | ((uint16_t)in[in_len - 1u] << 8);
    if (crc != got)
    {
        return RES_PARAM;
    }

    payload = (uint16_t)(in_len - FRAME_OH);
    *cmd = in[2];
    *data = (payload == 0u) ? NULL : &in[3];
    *data_len = payload;
    return RES_OK;
}

const frame_driver_t frame_sw_driver =
{
    .pf_pack   = frame_pack,
    .pf_unpack = frame_unpack,
};
