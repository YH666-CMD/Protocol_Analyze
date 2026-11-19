#include "app_spi_analyze.h"
#include "../../03_Driver/spi/drv_spi.h"
#include <stddef.h>

result_t app_spi_cfg(const uint8_t *data, uint16_t len)
{
    result_t rc;
    uint16_t n;

    if (data == NULL || len < 2u)
    {
        return RES_PARAM;
    }
    if (data[0] > 3u)
    {
        return RES_PARAM;
    }
    if (data[1] == 0u)
    {
        if (len != 2u)
        {
            return RES_PARAM;
        }
        return drv_spi.pf_mode(data[0]);
    }
    if (data[1] != 1u)
    {
        return RES_PARAM;
    }
    n = (uint16_t)(len - 2u);
    if (n == 0u || n > SPI_XFER_MAX)
    {
        return RES_PARAM;
    }
    rc = drv_spi.pf_mode(data[0]);
    if (rc != RES_OK)
    {
        return rc;
    }
    return drv_spi.pf_xfer(&data[2], n);
}
