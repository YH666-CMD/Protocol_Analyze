#include "drv_adc.h"
#include "pin_map.h"
#include "dma_config.h"
#include "ch32v20x.h"

/*
 * 只写要用的寄存器，不调用 ADC_Init / DMA_Init，避免把整份库链进来。
 * 采样时间用 239.5 个 ADC 时钟，和官方例程一样，不再缩短。
 * ADC 时钟是 PCLK2 的 8 分频。96MHz 时大约 12MHz。
 * 校准按本芯片 D6 的做法：采 10 次，取中间一段再合成偏移。
 * 半区在 DMA 中断里拷进队列。USB 任务 10 毫秒来取，不再只留最新一块。
 */
#if (DMA_ID_ADC1 != 1u)
#error "ADC DMA must stay on DMA1 channel 1"
#endif

#if !defined(CH32V20x_D6)
#error "ADC calibration below is the D6 formula used by CH32V203C8"
#endif

#if ((PP_HALF_SIZE % ADC_CH_NUM) != 0u)
#error "ADC half buffer must divide evenly by 3 channels"
#endif

#define ADC_SMP_2395     7u
#define ADC_CAL_SPIN     2000u
#define ADC_TRIG_SW      0x000E0000u
#define ADC_Q_SLOTS      10u

static volatile uint8_t s_on;
static int16_t s_cal;
static pp_t s_pp;
static uint16_t s_q[ADC_Q_SLOTS][PP_HALF_SIZE];
static volatile uint8_t s_q_w;
static volatile uint8_t s_q_r;
static volatile uint8_t s_q_n;
static volatile uint32_t s_q_drop;

static void adc_q_clear(void);

static uint32_t adc_irq_lock(void)
{
    uint32_t on;

    on = NVIC_GetStatusIRQ(DMA1_Channel1_IRQn);
    NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    return on;
}

static void adc_irq_unlock(uint32_t state)
{
    if (state != 0u)
    {
        NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    }
}

static void adc_pins_idle(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = BOARD_ADC_CH0_PIN | BOARD_ADC_CH1_PIN | BOARD_ADC_CH2_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(BOARD_ADC_PORT, &gpio);
}

static uint8_t adc_wait_clear(uint32_t bit)
{
    uint16_t n;

    for (n = 0u; n < ADC_CAL_SPIN; n++)
    {
        if ((ADC1->CTLR2 & bit) == 0u)
        {
            return 1u;
        }
    }
    return 0u;
}

static uint8_t adc_cal_one(uint16_t *out)
{
    ADC1->CTLR2 |= ADC_RSTCAL;
    if (adc_wait_clear(ADC_RSTCAL) == 0u)
    {
        return 0u;
    }
    ADC1->CTLR2 |= ADC_CAL;
    if (adc_wait_clear(ADC_CAL) == 0u)
    {
        return 0u;
    }
    *out = (uint16_t)ADC1->RDATAR;
    return 1u;
}

static void adc_sort(uint16_t *buf, uint8_t n)
{
    uint8_t i;
    uint8_t j;

    for (i = 1u; i < n; i++)
    {
        uint16_t v;

        v = buf[i];
        j = i;
        while (j > 0u && buf[j - 1u] > v)
        {
            buf[j] = buf[j - 1u];
            j--;
        }
        buf[j] = v;
    }
}

/* 和库里 D6 的 Get_CalibrationValue 同一条式子，不把那份库链进来。 */
static int16_t adc_cal_value(const uint16_t *buf)
{
    uint16_t t;
    uint16_t p;
    uint8_t i;
    uint8_t j;
    int16_t a;
    int16_t b;

    t = 0u;
    p = 0u;
    i = 0u;
    while (i < 6u && buf[i + 2u] <= 1536u)
    {
        t = (uint16_t)(t + buf[i + 2u]);
        i++;
    }
    if (i > 0u)
    {
        t = (uint16_t)((t / i) + (((t % i) * 2u) / i));
    }
    else
    {
        t = 1024u;
    }

    j = (uint8_t)(6u - i);
    if (j > 0u)
    {
        while (i < 6u)
        {
            p = (uint16_t)(p + buf[i + 2u]);
            i++;
        }
        p = (uint16_t)((p / j) + (((p % j) * 2u) / j));
    }
    else
    {
        p = 2048u;
    }

    a = (int16_t)(1024 - (int16_t)t);
    b = (int16_t)(2048 - (int16_t)p);
    return (int16_t)(((a + b) / 2) + ((a + b) % 2));
}

static uint8_t adc_calibrate(void)
{
    uint16_t buf[10];
    uint8_t i;

    for (i = 0u; i < 10u; i++)
    {
        if (adc_cal_one(&buf[i]) == 0u)
        {
            return 0u;
        }
    }
    adc_sort(buf, 10u);
    s_cal = adc_cal_value(buf);
    return 1u;
}

static uint16_t adc_apply(uint16_t raw)
{
    int32_t v;

    if (raw == 0u)
    {
        return 0u;
    }
    if (raw >= 4095u)
    {
        return 4095u;
    }
    v = (int32_t)raw + (int32_t)s_cal;
    if (v < 0)
    {
        return 0u;
    }
    if (v > 4095)
    {
        return 4095u;
    }
    return (uint16_t)v;
}

static void adc_dma_arm(void)
{
    DMA1_Channel1->CFGR &= (uint32_t)~DMA_CFGR1_EN;
    DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
    DMA1_Channel1->MADDR = (uint32_t)s_pp.buf;
    DMA1_Channel1->CNTR = PP_BUF_SIZE;
    DMA1_Channel1->CFGR = DMA_CFGR1_MINC | DMA_CFGR1_CIRC |
                          DMA_CFGR1_PSIZE_0 | DMA_CFGR1_MSIZE_0 |
                          DMA_CFGR1_PL_0 | DMA_CFGR1_HTIE | DMA_CFGR1_TCIE;
    DMA1->INTFCR = DMA_CGIF1 | DMA_CTCIF1 | DMA_CHTIF1 | DMA_CTEIF1;
    DMA1_Channel1->CFGR |= DMA_CFGR1_EN;
}

static void adc_stop(void);

static result_t adc_start(void)
{
    uint32_t samp;

    if (s_on != 0u)
    {
        return RES_OK;
    }

    pp_sw_driver.pf_pp_init(&s_pp, adc_irq_lock, adc_irq_unlock);
    adc_pins_idle();
    RCC->CFGR0 = (RCC->CFGR0 & ~RCC_ADCPRE) | RCC_ADCPRE_DIV8;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    ADC1->CTLR2 &= (uint32_t)~ADC_ADON;
    ADC1->CTLR1 = ADC_SCAN;
    ADC1->CTLR2 = ADC_CONT | ADC_TRIG_SW;
    ADC1->RSQR1 = (uint32_t)(ADC_CH_NUM - 1u) << 20;
    ADC1->RSQR2 = 0u;
    ADC1->RSQR3 = (uint32_t)(0u | (1u << 5) | (2u << 10));
    samp = (ADC_SMP_2395) | (ADC_SMP_2395 << 3) | (ADC_SMP_2395 << 6);
    ADC1->SAMPTR2 = samp;
    ADC1->SAMPTR1 = 0u;
    ADC1->CTLR2 |= ADC_ADON;
    if (adc_calibrate() == 0u)
    {
        adc_stop();
        return RES_OVERRUN;
    }

    adc_q_clear();
    s_q_drop = 0u;
    s_on = 1u;
    adc_dma_arm();
    ADC1->CTLR2 |= ADC_DMA;
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    ADC1->CTLR2 |= ADC_EXTTRIG | ADC_SWSTART;
    return RES_OK;
}

static void adc_stop(void)
{
    NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    NVIC_DisableIRQ(ADC_IRQn);
    if ((RCC->AHBPCENR & RCC_AHBPeriph_DMA1) != 0u)
    {
        DMA1_Channel1->CFGR &= (uint32_t)~DMA_CFGR1_EN;
        DMA1_Channel1->CNTR = 0u;
        DMA1->INTFCR = DMA_CGIF1 | DMA_CTCIF1 | DMA_CHTIF1 | DMA_CTEIF1;
    }
    if ((RCC->APB2PCENR & RCC_APB2Periph_ADC1) != 0u)
    {
        ADC1->CTLR2 &= (uint32_t)~(ADC_ADON | ADC_DMA | ADC_CONT | ADC_SWSTART | ADC_EXTTRIG);
    }
    adc_pins_idle();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    s_on = 0u;
    adc_q_clear();
}

static void adc_q_clear(void)
{
    uint32_t state;

    state = adc_irq_lock();
    s_q_w = 0u;
    s_q_r = 0u;
    s_q_n = 0u;
    adc_irq_unlock(state);
}

/* 只在 DMA 中断里调用。队列满就丢掉最旧的一块。 */
static void adc_q_push(const uint16_t *src)
{
    uint8_t slot;
    uint16_t i;

    if (s_on == 0u || src == NULL)
    {
        return;
    }
    if (s_q_n >= ADC_Q_SLOTS)
    {
        s_q_r = (uint8_t)((s_q_r + 1u) % ADC_Q_SLOTS);
        s_q_n--;
        if (s_q_drop < 0xFFFFFFFFu)
        {
            s_q_drop++;
        }
    }
    slot = s_q_w;
    for (i = 0u; i < PP_HALF_SIZE; i++)
    {
        s_q[slot][i] = src[i];
    }
    s_q_w = (uint8_t)((slot + 1u) % ADC_Q_SLOTS);
    s_q_n++;
}

static bool adc_take(pp_sample_t *out, uint16_t *rounds)
{
    uint32_t state;
    uint16_t raw[PP_HALF_SIZE];
    uint16_t per_ch;
    uint16_t n;
    uint16_t i;
    uint8_t ch;
    uint8_t slot;

    if (rounds != NULL)
    {
        *rounds = 0u;
    }
    if (out == NULL)
    {
        return false;
    }
    state = adc_irq_lock();
    if (s_on == 0u || s_q_n == 0u)
    {
        adc_irq_unlock(state);
        return false;
    }
    slot = s_q_r;
    for (i = 0u; i < PP_HALF_SIZE; i++)
    {
        raw[i] = s_q[slot][i];
    }
    s_q_r = (uint8_t)((slot + 1u) % ADC_Q_SLOTS);
    s_q_n--;
    adc_irq_unlock(state);

    per_ch = (uint16_t)(PP_HALF_SIZE / ADC_CH_NUM);
    for (n = 0u; n < per_ch; n++)
    {
        for (ch = 0u; ch < ADC_CH_NUM; ch++)
        {
            out[(uint16_t)ch * per_ch + n] = adc_apply(raw[n * ADC_CH_NUM + ch]);
        }
    }
    if (rounds != NULL)
    {
        *rounds = per_ch;
    }
    return true;
}

void DMA1_Channel1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void DMA1_Channel1_IRQHandler(void)
{
    uint32_t flags;

    flags = DMA1->INTFR;
    if ((flags & DMA_HTIF1) != 0u)
    {
        DMA1->INTFCR = DMA_CHTIF1;
        adc_q_push(s_pp.buf);
    }
    if ((flags & DMA_TCIF1) != 0u)
    {
        DMA1->INTFCR = DMA_CTCIF1;
        adc_q_push(&s_pp.buf[PP_HALF_SIZE]);
    }
}

const drv_adc_driver_t drv_adc =
{
    .pf_start = adc_start,
    .pf_stop  = adc_stop,
    .pf_take  = adc_take,
};
