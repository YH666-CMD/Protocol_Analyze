#include "app_heartbeat.h"
#include "bsp_gpio.h"

volatile uint32_t g_app_heartbeat_count;

void app_heartbeat_init(void)
{
    g_app_heartbeat_count = 0u;
}

void app_heartbeat_on_tick(uint32_t now_ms, uint32_t elapsed_ms)
{
    (void)now_ms;
    (void)elapsed_ms;

    g_app_heartbeat_count++;
    bsp_gpio.pf_toggle(BSP_PIN_HEARTBEAT);
}
