#include "app_i2c_parse.h"
#include "../../03_Driver/i2c/drv_i2c.h"
#include <stddef.h>

result_t app_i2c_cfg(const uint8_t *data, uint16_t len)
{
    uint16_t n;

    if (data == NULL || len < 3u)
    {
        return RES_PARAM;
    }
    if (data[0] > 1u || data[1] > 0x7Fu)
    {
        return RES_PARAM;
    }
    if (data[2] == 0u)
    {
        if (len != 3u)
        {
            return RES_PARAM;
        }
        return drv_i2c.pf_role(data[0], data[1]);
    }
    if (data[2] == 1u)
    {
        return drv_i2c.pf_scan();
    }
    if (data[2] == 2u)
    {
        n = (uint16_t)(len - 3u);
        if (n == 0u || n > I2C_IO_MAX)
        {
            return RES_PARAM;
        }
        if (drv_i2c.pf_role(data[0], data[1]) != RES_OK)
        {
            return RES_STATE;
        }
        return drv_i2c.pf_write(&data[3], n);
    }
    if (data[2] == 3u)
    {
        if (data[0] != 0u || len != 4u || data[3] == 0u || data[3] > I2C_IO_MAX)
        {
            return RES_PARAM;
        }
        if (drv_i2c.pf_role(0u, data[1]) != RES_OK)
        {
            return RES_STATE;
        }
        return drv_i2c.pf_read(data[3]);
    }
    if (data[2] == 4u)
    {
        if (len != 4u || data[3] > 1u)
        {
            return RES_PARAM;
        }
        return drv_i2c.pf_speed(data[3]);
    }
    return RES_PARAM;
}
