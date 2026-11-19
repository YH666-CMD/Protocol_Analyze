#include "drv_adc.h"
#include "pin_map.h"
#include "dma_config.h"
#include "ch32v20x.h"

/*
 * 采样还没写。打开只记标志，避免把整份 ADC 库链进来。
 * 关闭停掉 DMA1 通道 1。通道 5 是串口接收，这里不动，DMA 总时钟也不关。
 */
#if (DMA_ID_ADC1 != 1u)
#error "ADC DMA must stay on DMA1 channel 1"
#endif

static uint8_t s_on;

static void adc_pins_idle(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = BOARD_ADC_CH0_PIN | BOARD_ADC_CH1_PIN | BOARD_ADC_CH2_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(BOARD_ADC_PORT, &gpio);
}

static result_t adc_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    s_on = 1u;
    return RES_OK;
}

static void adc_stop(void)
{
    if ((RCC->AHBPCENR & RCC_AHBPeriph_DMA1) != 0u)
    {
        DMA1_Channel1->CFGR &= (uint32_t)~DMA_CFGR1_EN;
        DMA1_Channel1->CNTR = 0u;
    }
    NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    NVIC_DisableIRQ(ADC_IRQn);
    if ((RCC->APB2PCENR & RCC_APB2Periph_ADC1) != 0u)
    {
        ADC1->CTLR2 &= (uint32_t)~(ADC_ADON | ADC_DMA);
    }
    adc_pins_idle();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    s_on = 0u;
}

const drv_adc_driver_t drv_adc =
{
    .pf_start = adc_start,
    .pf_stop  = adc_stop,
};
