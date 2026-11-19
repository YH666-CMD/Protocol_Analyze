#include "drv_pwm_in.h"
#include "pin_map.h"
#include "ch32v20x.h"

typedef char pwm_in_pin_check[(BOARD_PWM_IN_PIN == GPIO_Pin_15) ? 1 : -1];

/*
 * TIM2 在 APB1 二分频时时钟是 96MHz。PSC = 95，一格 1 微秒。
 * 通道 1 上升沿记周期，通道 2 下降沿记高电平，计数到边沿就清零。
 * 不进中断。PA15 默认是 JTAG，这里只关掉 JTAG，SWD 仍在 PA13/PA14。
 */
#define PWM_IN_PSC       95u

static uint8_t s_on;
static uint8_t s_have;
static uint8_t s_rep;
static uint16_t s_per;
static uint16_t s_hi;

static void pwm_pin(void)
{
    GPIO_InitTypeDef gpio;
    uint32_t map;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    map = AFIO->PCFR1;
    map &= ~(AFIO_PCFR1_TIM2_REMAP | AFIO_PCFR1_SWJ_CFG);
    map |= AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1 | AFIO_PCFR1_SWJ_CFG_JTAGDISABLE;
    AFIO->PCFR1 = map;
    gpio.GPIO_Pin = BOARD_PWM_IN_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BOARD_PWM_IN_PORT, &gpio);
}

static result_t pwm_in_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    pwm_pin();
    TIM2->CTLR1 &= (uint16_t)~TIM_CEN;
    TIM2->PSC = PWM_IN_PSC;
    TIM2->ATRLR = 65535u;
    TIM2->CHCTLR1 = (uint16_t)(TIM_CC1S_0 | TIM_CC2S_1);
    TIM2->CCER = (uint16_t)(TIM_CC1E | TIM_CC2E | TIM_CC2P);
    TIM2->SMCFGR = (uint16_t)(TIM_SMS_2 | TIM_TS_2 | TIM_TS_0);
    TIM2->INTFR = 0u;
    TIM2->CNT = 0u;
    TIM2->CTLR1 |= TIM_CEN;
    s_have = 0u;
    s_rep = 0u;
    s_on = 1u;
    return RES_OK;
}

static void pwm_in_stop(void)
{
    NVIC_DisableIRQ(TIM2_IRQn);
    if ((RCC->APB1PCENR & RCC_APB1Periph_TIM2) != 0u)
    {
        TIM2->CCER = 0u;
        TIM2->CTLR1 &= (uint16_t)~TIM_CEN;
    }
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
    s_on = 0u;
    s_have = 0u;
    s_rep = 0u;
}

static void pwm_in_poll(void)
{
    uint16_t flag;
    uint16_t per;
    uint16_t hi;

    if (s_on == 0u)
    {
        return;
    }
    flag = TIM2->INTFR;
    if ((flag & TIM_CC1IF) != 0u)
    {
        per = TIM2->CH1CVR;
        hi = TIM2->CH2CVR;
        TIM2->INTFR = (uint16_t)~(TIM_CC1IF | TIM_CC2IF | TIM_UIF);
        if (per != 0u && (s_have == 0u || per != s_per || hi != s_hi))
        {
            s_per = per;
            s_hi = hi;
            s_have = 1u;
            s_rep = 1u;
        }
        return;
    }
    if ((flag & TIM_UIF) != 0u)
    {
        TIM2->INTFR = (uint16_t)~TIM_UIF;
        if (s_have != 0u)
        {
            s_per = 0u;
            s_hi = 0u;
            s_have = 0u;
            s_rep = 1u;
        }
    }
}

static void pwm_put(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8);
    dst[2] = 0u;
    dst[3] = 0u;
}

static bool pwm_in_take(uint8_t *dst)
{
    if (dst == 0 || s_rep == 0u)
    {
        return false;
    }
    s_rep = 0u;
    pwm_put(dst, s_per);
    pwm_put(dst + 4, s_hi);
    return true;
}

const drv_pwm_in_driver_t drv_pwm_in =
{
    .pf_start = pwm_in_start,
    .pf_stop  = pwm_in_stop,
    .pf_poll  = pwm_in_poll,
    .pf_take  = pwm_in_take,
};
