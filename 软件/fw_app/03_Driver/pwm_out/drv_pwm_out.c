#include "drv_pwm_out.h"
#include "pin_map.h"
#include "ch32v20x.h"

/* 定时器时钟是 APB2 的 96MHz。PSC、ARR、CCR 由上位机算好。PWM 模式 1，高电平有效。 */
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

static result_t pwm_out_set(uint16_t psc, uint16_t arr, uint16_t ccr)
{
    GPIO_InitTypeDef gpio;

    if (s_on == 0u)
    {
        return RES_STATE;
    }
    if (arr == 0u)
    {
        return RES_PARAM;
    }
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);
    gpio.GPIO_Pin = BOARD_PWM_OUT_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(BOARD_PWM_OUT_PORT, &gpio);
    TIM1->CTLR1 = TIM_ARPE;
    TIM1->PSC = psc;
    TIM1->ATRLR = arr;
    TIM1->CH1CVR = ccr;
    TIM1->CHCTLR1 = (uint16_t)(TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE);
    TIM1->CCER = TIM_CC1E;
    TIM1->BDTR = TIM_MOE;
    TIM1->SWEVGR = TIM_UG;
    TIM1->CTLR1 = (uint16_t)(TIM_ARPE | TIM_CEN);
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
    .pf_set   = pwm_out_set,
    .pf_stop  = pwm_out_stop,
};
