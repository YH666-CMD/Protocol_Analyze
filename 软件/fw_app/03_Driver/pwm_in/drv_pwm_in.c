#include "drv_pwm_in.h"
#include "pin_map.h"
#include "ch32v20x.h"

typedef char pwm_in_pin_check[(BOARD_PWM_IN_PIN == GPIO_Pin_15) ? 1 : -1];

/*
 * 捕获还没写。打开只记标志。
 * PA15 现在不重映射。关闭只停 TIM2，不改调试口。
 */
static uint8_t s_on;

static result_t pwm_in_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    s_on = 1u;
    return RES_OK;
}

static void pwm_in_stop(void)
{
    if ((RCC->APB1PCENR & RCC_APB1Periph_TIM2) != 0u)
    {
        TIM2->CCER = 0u;
        TIM2->CTLR1 &= (uint16_t)~TIM_CEN;
    }
    NVIC_DisableIRQ(TIM2_IRQn);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
    s_on = 0u;
}

const drv_pwm_in_driver_t drv_pwm_in =
{
    .pf_start = pwm_in_start,
    .pf_stop  = pwm_in_stop,
};
