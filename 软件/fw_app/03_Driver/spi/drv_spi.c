#include "drv_spi.h"
#include "pin_map.h"
#include "ch32v20x.h"

/*
 * 收发还没写。打开只记标志，避免把整份 SPI 库链进来。
 * 关闭一定把时钟和中断停掉，NSS 拉高松开。
 */
static uint8_t s_on;

static void spi_nss_release(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = BOARD_SPI_NSS_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BOARD_SPI_NSS_PORT, &gpio);
    GPIO_SetBits(BOARD_SPI_NSS_PORT, BOARD_SPI_NSS_PIN);
}

static result_t spi_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    s_on = 1u;
    return RES_OK;
}

static void spi_stop(void)
{
    if ((RCC->APB2PCENR & RCC_APB2Periph_SPI1) != 0u)
    {
        SPI1->CTLR1 &= (uint16_t)~SPI_CTLR1_SPE;
    }
    NVIC_DisableIRQ(SPI1_IRQn);
    spi_nss_release();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
    s_on = 0u;
}

const drv_spi_driver_t drv_spi =
{
    .pf_start = spi_start,
    .pf_stop  = spi_stop,
};
