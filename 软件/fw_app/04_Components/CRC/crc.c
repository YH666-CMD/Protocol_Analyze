#include "crc.h"

/* 算法参数只在本文件使用，不泄漏到接口 */
typedef struct
{
    uint8_t  width;
    uint32_t poly;
    uint32_t init;
    uint32_t xorout;
    uint8_t  refin;
    uint8_t  refout;
} crc_param_t;

static uint32_t crc_reflect(uint32_t data, uint8_t bits)
{
    uint32_t reflected = 0;
    uint8_t i;

    for(i = 0; i < bits; i++)
    {
        if((data & 1u) != 0u)
        {
            reflected |= 1u << (bits - 1u - i);
        }
        data >>= 1;
    }

    return reflected;
}

static uint32_t crc_calc(const crc_param_t *param, const uint8_t *data, uint32_t len)
{
    uint32_t mask;
    uint32_t top;
    uint32_t crc;
    uint32_t poly;
    uint32_t i;
    uint8_t bit;

    if(param->width >= 32u)
    {
        mask = 0xFFFFFFFFu;
    }
    else
    {
        mask = (1u << param->width) - 1u;
    }

    top = 1u << (param->width - 1u);
    crc = param->init & mask;
    poly = param->poly & mask;

    if(param->refin != 0u)
    {
        crc = crc_reflect(crc, param->width);
        poly = crc_reflect(poly, param->width);
    }

    if(data == 0)
    {
        len = 0;
    }

    for(i = 0; i < len; i++)
    {
        if(param->refin != 0u)
        {
            crc ^= data[i];
            for(bit = 0; bit < 8u; bit++)
            {
                if((crc & 1u) != 0u)
                {
                    crc = (crc >> 1) ^ poly;
                }
                else
                {
                    crc >>= 1;
                }
                crc &= mask;
            }
        }
        else
        {
            crc ^= (uint32_t)data[i] << (param->width - 8u);
            for(bit = 0; bit < 8u; bit++)
            {
                if((crc & top) != 0u)
                {
                    crc = (crc << 1) ^ poly;
                }
                else
                {
                    crc <<= 1;
                }
                crc &= mask;
            }
        }
    }

    if(param->refout != param->refin)
    {
        crc = crc_reflect(crc, param->width);
    }

    crc ^= param->xorout;
    return crc & mask;
}

static uint8_t crc8_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {8u, 0x07u, 0x00u, 0x00u, 0u, 0u};

    return (uint8_t)crc_calc(&param, data, len);
}

static uint8_t crc8_maxim_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {8u, 0x31u, 0x00u, 0x00u, 1u, 1u};

    return (uint8_t)crc_calc(&param, data, len);
}

static uint16_t crc16_modbus_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {16u, 0x8005u, 0xFFFFu, 0x0000u, 1u, 1u};

    return (uint16_t)crc_calc(&param, data, len);
}

static uint16_t crc16_ccitt_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {16u, 0x1021u, 0xFFFFu, 0x0000u, 0u, 0u};

    return (uint16_t)crc_calc(&param, data, len);
}

static uint16_t crc16_xmodem_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {16u, 0x1021u, 0x0000u, 0x0000u, 0u, 0u};

    return (uint16_t)crc_calc(&param, data, len);
}

static uint32_t crc32_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {32u, 0x04C11DB7u, 0xFFFFFFFFu, 0xFFFFFFFFu, 1u, 1u};

    return crc_calc(&param, data, len);
}

static uint32_t crc32_mpeg2_calc(const uint8_t *data, uint32_t len)
{
    static const crc_param_t param = {32u, 0x04C11DB7u, 0xFFFFFFFFu, 0x00000000u, 0u, 0u};

    return crc_calc(&param, data, len);
}

const crc_driver_interface_t crc_sw_driver =
{
    .pf_crc8          = crc8_calc,
    .pf_crc8_maxim    = crc8_maxim_calc,
    .pf_crc16_modbus  = crc16_modbus_calc,
    .pf_crc16_ccitt   = crc16_ccitt_calc,
    .pf_crc16_xmodem  = crc16_xmodem_calc,
    .pf_crc32         = crc32_calc,
    .pf_crc32_mpeg2   = crc32_mpeg2_calc,
};
