#include "drv_pwm_out.h"
#include "pin_map.h"
#include "ch32v20x.h"

/* 波形还没写。打开只记标志。关闭停比较输出，PA8 改成浮空输入。 */
static uint8_t s_on;

static void pwm_pin_idle(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = BOARD_PWM_OUT_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BOARD_PWM_OUT_PORT, &gpio);
}

static result_t pwm_out_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    s_on = 1u;
    return RES_OK;
}

static void pwm_out_stop(void)
{
    if ((RCC->APB2PCENR & RCC_APB2Periph_TIM1) != 0u)
    {
        TIM1->BDTR &= (uint16_t)~TIM_MOE;
        TIM1->CCER = 0u;
        TIM1->CTLR1 &= (uint16_t)~TIM_CEN;
    }
    NVIC_DisableIRQ(TIM1_BRK_IRQn);
    NVIC_DisableIRQ(TIM1_UP_IRQn);
    NVIC_DisableIRQ(TIM1_TRG_COM_IRQn);
    NVIC_DisableIRQ(TIM1_CC_IRQn);
    pwm_pin_idle();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);
    s_on = 0u;
}

const drv_pwm_out_driver_t drv_pwm_out =
{
    .pf_start = pwm_out_start,
    .pf_stop  = pwm_out_stop,
};
