#include "app_pwm_gen.h"
#include "../../03_Driver/pwm_out/drv_pwm_out.h"
#include <stddef.h>

/* 6 字节：小端 PSC、ARR、CCR。频率和占空比不在这里算。 */
result_t app_pwm_out(const uint8_t *data, uint16_t len)
{
    uint16_t psc;
    uint16_t arr;
    uint16_t ccr;

    if (data == NULL || len != 6u)
    {
        return RES_PARAM;
    }
    psc = (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
    arr = (uint16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
    ccr = (uint16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));
    return drv_pwm_out.pf_set(psc, arr, ccr);
}
