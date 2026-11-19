#include "drv_i2c.h"
#include "pin_map.h"
#include "ch32v20x.h"

/*
 * 主从收发还没写。打开只记标志，避免把整份 I2C 库链进来。
 * 关闭关事件中断和错误中断，SCL/SDA 开漏拉高，等于松开总线。
 */
static uint8_t s_on;

static void i2c_bus_release(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = BOARD_I2C_SCL_PIN | BOARD_I2C_SDA_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(BOARD_I2C_SCL_PORT, &gpio);
    GPIO_SetBits(BOARD_I2C_SCL_PORT, BOARD_I2C_SCL_PIN | BOARD_I2C_SDA_PIN);
}

static result_t i2c_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    s_on = 1u;
    return RES_OK;
}

static void i2c_stop(void)
{
    if ((RCC->APB1PCENR & RCC_APB1Periph_I2C1) != 0u)
    {
        I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_PE;
    }
    NVIC_DisableIRQ(I2C1_EV_IRQn);
    NVIC_DisableIRQ(I2C1_ER_IRQn);
    i2c_bus_release();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    s_on = 0u;
}

const drv_i2c_driver_t drv_i2c =
{
    .pf_start = i2c_start,
    .pf_stop  = i2c_stop,
};
