#include "bsp_gpio.h"
#include "bsp_clk.h"
#include "pin_map.h"
#include "ch32v20x.h"

static bool gpio_is_heartbeat(bsp_pin_id_t id)
{
#if BOARD_HEARTBEAT_ENABLE
    return (id == BSP_PIN_HEARTBEAT);
#else
    (void)id;
    return false;
#endif
}

static void gpio_init(void)
{
#if BOARD_HEARTBEAT_ENABLE
    GPIO_InitTypeDef gpio;

    bsp_clk.pf_gpio_clock_on(BSP_PIN_HEARTBEAT);

    gpio.GPIO_Pin = BOARD_HEARTBEAT_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BOARD_HEARTBEAT_PORT, &gpio);
    GPIO_ResetBits(BOARD_HEARTBEAT_PORT, BOARD_HEARTBEAT_PIN);
#endif
}

static void gpio_write(bsp_pin_id_t id, bool level_high)
{
    if (!gpio_is_heartbeat(id)) return;

#if BOARD_HEARTBEAT_ENABLE
    GPIO_WriteBit(BOARD_HEARTBEAT_PORT, BOARD_HEARTBEAT_PIN,
                  level_high ? Bit_SET : Bit_RESET);
#else
    (void)level_high;
#endif
}

static void gpio_toggle(bsp_pin_id_t id)
{
    if (!gpio_is_heartbeat(id)) return;

#if BOARD_HEARTBEAT_ENABLE
    {
        uint8_t level;

        level = GPIO_ReadOutputDataBit(BOARD_HEARTBEAT_PORT, BOARD_HEARTBEAT_PIN);
        GPIO_WriteBit(BOARD_HEARTBEAT_PORT, BOARD_HEARTBEAT_PIN,
                      (level == 0u) ? Bit_SET : Bit_RESET);
    }
#endif
}

static bool gpio_read(bsp_pin_id_t id)
{
    if (!gpio_is_heartbeat(id)) return false;

#if BOARD_HEARTBEAT_ENABLE
    return (GPIO_ReadOutputDataBit(BOARD_HEARTBEAT_PORT, BOARD_HEARTBEAT_PIN) != 0u);
#else
    return false;
#endif
}

const bsp_gpio_driver_t bsp_gpio =
{
    .pf_init   = gpio_init,
    .pf_write  = gpio_write,
    .pf_toggle = gpio_toggle,
    .pf_read   = gpio_read,
};
