#include "app_heartbeat.h"
#include "bsp_gpio.h"

volatile uint32_t g_app_heartbeat_count;

result_t app_heartbeat_init(void)
{
    g_app_heartbeat_count = 0u;
    return RES_OK;
}

result_t app_heartbeat_on_tick(uint32_t now_ms, uint32_t elapsed_ms, uint32_t event)
{
    (void)now_ms;
    (void)elapsed_ms;
    (void)event;

    g_app_heartbeat_count++;
    bsp_gpio.pf_toggle(BSP_PIN_HEARTBEAT);
    return RES_OK;
}
