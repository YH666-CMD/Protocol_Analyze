#include "bsp_clk.h"
#include "pin_map.h"
#include "ch32v20x.h"

#if BOARD_HEARTBEAT_ENABLE
static uint32_t clk_gpio_periph(GPIO_TypeDef *port)
{
    if (port == GPIOA) return RCC_APB2Periph_GPIOA;
    if (port == GPIOB) return RCC_APB2Periph_GPIOB;
    if (port == GPIOC) return RCC_APB2Periph_GPIOC;
    if (port == GPIOD) return RCC_APB2Periph_GPIOD;
    return 0u;
}
#endif

static void clk_init(void)
{
    SystemCoreClockUpdate();
}

static uint32_t clk_core_hz(void)
{
    return SystemCoreClock;
}

static void clk_gpio_clock_on(bsp_pin_id_t id)
{
#if BOARD_HEARTBEAT_ENABLE
    uint32_t periph;

    if (id != BSP_PIN_HEARTBEAT) return;

    periph = clk_gpio_periph(BOARD_HEARTBEAT_PORT);
    if (periph == 0u) return;

    RCC_APB2PeriphClockCmd(periph, ENABLE);
#else
    (void)id;
#endif
}

const bsp_clk_driver_t bsp_clk =
{
    .pf_init          = clk_init,
    .pf_core_hz       = clk_core_hz,
    .pf_gpio_clock_on = clk_gpio_clock_on,
};
