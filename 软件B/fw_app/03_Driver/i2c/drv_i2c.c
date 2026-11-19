#include "drv_i2c.h"
#include "pin_map.h"
#include "ch32v20x.h"

/*
 * APB1 固定 48MHz。100kHz 时 CCR = 240。400kHz 快速模式、占空 2:1 时 CCR = 40。
 * 改速率先关掉 PE，写完再打开，当前这次配置立刻作用到下一次传输。
 * 不调用 I2C_Init，避免把整份库链进来。
 * 主机只读 PB4。从机用推挽拉低；缓冲里还有没读走的字节时，STOP 之后也不拉高。
 */
#define I2C_PCLK_MHZ     48u
#define I2C_CCR_100K     240u
#define I2C_CCR_400K     40u
#define I2C_TRISE_400K   15u
#define I2C_SPIN         30000u
#define I2C_SCAN_FIRST   0x08u
#define I2C_SCAN_LAST    0x77u
#define I2C_SCAN_STEP    4u
#define I2C_HIT_MAX      32u

static uint8_t s_on;
static volatile uint8_t s_slave;
static volatile uint8_t s_addr;
static uint8_t s_rd_len;
static uint8_t s_fast;
static volatile uint8_t s_busy;
static volatile uint8_t s_done;
static volatile uint8_t s_nack;
static volatile uint8_t s_reading;
static uint8_t s_scan;
static uint8_t s_scan_at;

static uint8_t s_tx[I2C_IO_MAX];
static volatile uint8_t s_tx_len;
static volatile uint8_t s_tx_pos;
static uint8_t s_rx[I2C_IO_MAX];
static volatile uint8_t s_need;
static volatile uint8_t s_got;

static uint8_t s_hit[I2C_HIT_MAX];
static uint8_t s_hit_n;
static uint8_t s_rep[1u + I2C_IO_MAX];
static uint8_t s_rep_n;

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

static void i2c_pins_bus(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    gpio.GPIO_Pin = BOARD_I2C_SCL_PIN | BOARD_I2C_SDA_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(BOARD_I2C_SCL_PORT, &gpio);
}

/* 主机不驱动通知脚。上拉输入，对方没拉时读到高。 */
static void i2c_pin_listen(void)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin = BOARD_I2C_NOTIFY_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(BOARD_I2C_NOTIFY_PORT, &gpio);
}

static void i2c_pin_drive(uint8_t high)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin = BOARD_I2C_NOTIFY_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BOARD_I2C_NOTIFY_PORT, &gpio);
    if (high != 0u)
    {
        GPIO_SetBits(BOARD_I2C_NOTIFY_PORT, BOARD_I2C_NOTIFY_PIN);
    }
    else
    {
        GPIO_ResetBits(BOARD_I2C_NOTIFY_PORT, BOARD_I2C_NOTIFY_PIN);
    }
}

static void i2c_slave_pin(void)
{
    if (s_slave == 0u)
    {
        return;
    }
    if (s_tx_pos < s_tx_len)
    {
        i2c_pin_drive(0u);
    }
    else
    {
        i2c_pin_drive(1u);
    }
}

static uint8_t i2c_pin_low(void)
{
    uint16_t n;

    for (n = 0u; n < 4000u; n++)
    {
        if (GPIO_ReadInputDataBit(BOARD_I2C_NOTIFY_PORT, BOARD_I2C_NOTIFY_PIN) != 0u)
        {
            return 0u;
        }
    }
    return 1u;
}

/* PE 为 0 才能改时钟。调用前总线时钟已经打开。 */
static void i2c_apply_clock(void)
{
    uint16_t ctlr;

    ctlr = I2C1->CTLR1;
    I2C1->CTLR1 = (uint16_t)(ctlr & (uint16_t)~I2C_CTLR1_PE);
    if (s_fast != 0u)
    {
        I2C1->CKCFGR = (uint16_t)(I2C_CCR_400K | I2C_CKCFGR_FS);
        I2C1->RTR = I2C_TRISE_400K;
    }
    else
    {
        I2C1->CKCFGR = I2C_CCR_100K;
        I2C1->RTR = (uint16_t)(I2C_PCLK_MHZ + 1u);
    }
    I2C1->CTLR1 = ctlr;
}

static void i2c_hw(uint8_t slave, uint8_t addr)
{
    i2c_pins_bus();
    I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_PE;
    I2C1->CTLR2 = (uint16_t)(I2C_PCLK_MHZ | I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITERREN);
    I2C1->OADDR1 = (uint16_t)((uint16_t)addr << 1);
    i2c_apply_clock();
    I2C1->CTLR1 = (uint16_t)(I2C_CTLR1_PE | I2C_CTLR1_ACK);
    s_slave = slave;
    s_addr = addr;
    s_tx_len = 0u;
    s_tx_pos = 0u;
    s_busy = 0u;
    s_done = 0u;
    s_scan = 0u;
    if (slave != 0u)
    {
        i2c_pin_drive(1u);
    }
    else
    {
        i2c_pin_listen();
    }
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_EnableIRQ(I2C1_ER_IRQn);
}

static void i2c_queue_rep(uint8_t tag, const uint8_t *src, uint8_t n)
{
    uint8_t i;

    if (s_rep_n != 0u || n > I2C_IO_MAX)
    {
        return;
    }
    s_rep[0] = tag;
    for (i = 0u; i < n; i++)
    {
        s_rep[1u + i] = src[i];
    }
    s_rep_n = (uint8_t)(1u + n);
}

static void i2c_clear_addr(void)
{
    (void)I2C1->STAR1;
    (void)I2C1->STAR2;
}

static void i2c_stop_now(void)
{
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
}

static result_t i2c_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    s_rd_len = I2C_RD_DEFAULT;
    s_rep_n = 0u;
    s_hit_n = 0u;
    i2c_hw(0u, 0u);
    s_on = 1u;
    return RES_OK;
}

static void i2c_stop(void)
{
    NVIC_DisableIRQ(I2C1_EV_IRQn);
    NVIC_DisableIRQ(I2C1_ER_IRQn);
    if ((RCC->APB1PCENR & RCC_APB1Periph_I2C1) != 0u)
    {
        I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_PE;
    }
    i2c_bus_release();
    i2c_pin_listen();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    s_on = 0u;
    s_busy = 0u;
    s_scan = 0u;
    s_tx_len = 0u;
    s_tx_pos = 0u;
}

static result_t i2c_role(uint8_t slave, uint8_t addr)
{
    if (s_on == 0u)
    {
        return RES_STATE;
    }
    if (slave > 1u || addr > 0x7Fu)
    {
        return RES_PARAM;
    }
    if (s_busy != 0u || s_scan != 0u)
    {
        return RES_BUSY;
    }
    if (s_slave == slave)
    {
        s_addr = addr;
        if (slave != 0u)
        {
            I2C1->OADDR1 = (uint16_t)((uint16_t)addr << 1);
        }
        return RES_OK;
    }
    NVIC_DisableIRQ(I2C1_EV_IRQn);
    NVIC_DisableIRQ(I2C1_ER_IRQn);
    I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_PE;
    i2c_bus_release();
    i2c_hw(slave, addr);
    return RES_OK;
}

static uint8_t i2c_wait_done(void)
{
    uint16_t n;

    for (n = 0u; n < I2C_SPIN; n++)
    {
        if (s_done != 0u)
        {
            return 1u;
        }
    }
    i2c_stop_now();
    s_busy = 0u;
    return 0u;
}

static void i2c_begin(uint8_t reading, uint8_t n)
{
    s_reading = reading;
    s_need = n;
    s_got = 0u;
    s_nack = 0u;
    s_done = 0u;
    s_busy = 1u;
    I2C1->CTLR1 |= I2C_CTLR1_ACK;
    I2C1->CTLR1 |= I2C_CTLR1_START;
}

static result_t i2c_write(const uint8_t *data, uint16_t len)
{
    uint8_t i;
    uint8_t room;

    if (s_on == 0u)
    {
        return RES_STATE;
    }
    if (data == NULL || len == 0u || len > I2C_IO_MAX)
    {
        return RES_PARAM;
    }
    if (s_slave != 0u)
    {
        if (s_tx_pos > 0u && s_tx_pos < s_tx_len)
        {
            for (i = 0u; i < (uint8_t)(s_tx_len - s_tx_pos); i++)
            {
                s_tx[i] = s_tx[s_tx_pos + i];
            }
            s_tx_len = (uint8_t)(s_tx_len - s_tx_pos);
            s_tx_pos = 0u;
        }
        else if (s_tx_pos >= s_tx_len)
        {
            s_tx_len = 0u;
            s_tx_pos = 0u;
        }
        room = (uint8_t)(I2C_IO_MAX - s_tx_len);
        if (len > room)
        {
            return RES_NO_MEM;
        }
        for (i = 0u; i < (uint8_t)len; i++)
        {
            s_tx[s_tx_len + i] = data[i];
        }
        s_tx_len = (uint8_t)(s_tx_len + (uint8_t)len);
        i2c_slave_pin();
        return RES_OK;
    }
    if (s_busy != 0u || s_scan != 0u)
    {
        return RES_BUSY;
    }
    for (i = 0u; i < (uint8_t)len; i++)
    {
        s_tx[i] = data[i];
    }
    s_tx_len = (uint8_t)len;
    s_tx_pos = 0u;
    i2c_begin(0u, s_tx_len);
    if (i2c_wait_done() == 0u)
    {
        return RES_OVERRUN;
    }
    s_busy = 0u;
    if (s_nack != 0u)
    {
        return RES_STATE;
    }
    return RES_OK;
}

static void i2c_finish_read(void)
{
    s_busy = 0u;
    if (s_nack == 0u && s_got != 0u)
    {
        i2c_queue_rep(0u, s_rx, s_got);
    }
}

static result_t i2c_read(uint8_t n)
{
    if (s_on == 0u || s_slave != 0u)
    {
        return RES_STATE;
    }
    if (n == 0u || n > I2C_IO_MAX)
    {
        return RES_PARAM;
    }
    if (s_busy != 0u || s_scan != 0u || s_rep_n != 0u)
    {
        return RES_BUSY;
    }
    s_rd_len = n;
    i2c_begin(1u, n);
    if (i2c_wait_done() == 0u)
    {
        return RES_OVERRUN;
    }
    i2c_finish_read();
    if (s_nack != 0u)
    {
        return RES_STATE;
    }
    return RES_OK;
}

static result_t i2c_scan(void)
{
    if (s_on == 0u || s_slave != 0u)
    {
        return RES_STATE;
    }
    if (s_busy != 0u || s_scan != 0u)
    {
        return RES_BUSY;
    }
    s_scan_at = I2C_SCAN_FIRST;
    s_hit_n = 0u;
    s_scan = 1u;
    return RES_OK;
}

static uint8_t i2c_flag(uint16_t mask, uint16_t spins)
{
    uint16_t n;

    for (n = 0u; n < spins; n++)
    {
        if ((I2C1->STAR1 & mask) != 0u)
        {
            return 1u;
        }
    }
    return 0u;
}

static uint8_t i2c_probe(uint8_t addr)
{
    uint16_t ctl;

    ctl = I2C1->CTLR2;
    I2C1->CTLR2 = (uint16_t)(ctl & (uint16_t)~(I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITERREN));
    I2C1->CTLR1 |= I2C_CTLR1_START;
    if (i2c_flag(I2C_STAR1_SB, 4000u) == 0u)
    {
        i2c_stop_now();
        I2C1->CTLR2 = ctl;
        return 0u;
    }
    I2C1->DATAR = (uint16_t)((uint16_t)addr << 1);
    if (i2c_flag((uint16_t)(I2C_STAR1_ADDR | I2C_STAR1_AF), 4000u) == 0u)
    {
        i2c_stop_now();
        I2C1->CTLR2 = ctl;
        return 0u;
    }
    if ((I2C1->STAR1 & I2C_STAR1_AF) != 0u)
    {
        I2C1->STAR1 &= (uint16_t)~I2C_STAR1_AF;
        i2c_stop_now();
        I2C1->CTLR2 = ctl;
        return 0u;
    }
    i2c_clear_addr();
    i2c_stop_now();
    I2C1->CTLR2 = ctl;
    return 1u;
}

static void i2c_poll(void)
{
    uint8_t n;
    uint8_t follow;

    if (s_on == 0u || s_rep_n != 0u)
    {
        return;
    }
    if (s_scan != 0u)
    {
        n = 0u;
        while (n < I2C_SCAN_STEP && s_scan_at <= I2C_SCAN_LAST)
        {
            if (i2c_probe(s_scan_at) != 0u && s_hit_n < I2C_HIT_MAX)
            {
                s_hit[s_hit_n] = s_scan_at;
                s_hit_n++;
            }
            s_scan_at++;
            n++;
        }
        if (s_scan_at > I2C_SCAN_LAST)
        {
            s_scan = 0u;
            i2c_queue_rep(1u, s_hit, s_hit_n);
        }
        return;
    }
    if (s_slave != 0u || s_busy != 0u)
    {
        return;
    }
    follow = 0u;
    while (follow < 2u && i2c_pin_low() != 0u)
    {
        i2c_begin(1u, s_rd_len);
        if (i2c_wait_done() == 0u)
        {
            return;
        }
        i2c_finish_read();
        follow++;
        if (s_rep_n != 0u)
        {
            return;
        }
    }
}

static bool i2c_take(uint8_t *dst, uint16_t cap, uint16_t *n)
{
    uint8_t i;

    if (n != NULL)
    {
        *n = 0u;
    }
    if (dst == NULL || s_rep_n == 0u || cap < s_rep_n)
    {
        return false;
    }
    for (i = 0u; i < s_rep_n; i++)
    {
        dst[i] = s_rep[i];
    }
    if (n != NULL)
    {
        *n = s_rep_n;
    }
    s_rep_n = 0u;
    return true;
}

void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void I2C1_EV_IRQHandler(void)
{
    uint16_t sr1;
    uint16_t sr2;
    uint8_t b;

    sr1 = I2C1->STAR1;
    if ((sr1 & I2C_STAR1_SB) != 0u)
    {
        I2C1->DATAR = (uint16_t)(((uint16_t)s_addr << 1) | s_reading);
        return;
    }
    if ((sr1 & I2C_STAR1_ADDR) != 0u)
    {
        if (s_slave == 0u && s_reading != 0u && s_need == 1u)
        {
            I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_ACK;
            i2c_stop_now();
        }
        sr2 = I2C1->STAR2;
        (void)sr1;
        if (s_slave != 0u && (sr2 & I2C_STAR2_TRA) != 0u)
        {
            if (s_tx_pos < s_tx_len)
            {
                I2C1->DATAR = s_tx[s_tx_pos];
                s_tx_pos++;
            }
            else
            {
                I2C1->DATAR = 0xFFu;
            }
        }
        return;
    }
    if ((sr1 & I2C_STAR1_BTF) != 0u && s_slave == 0u && s_reading == 0u &&
        s_busy != 0u && s_tx_pos >= s_tx_len)
    {
        i2c_stop_now();
        s_done = 1u;
        s_busy = 0u;
        return;
    }
    if ((sr1 & I2C_STAR1_TXE) != 0u && s_reading == 0u)
    {
        if (s_slave != 0u)
        {
            if (s_tx_pos < s_tx_len)
            {
                I2C1->DATAR = s_tx[s_tx_pos];
                s_tx_pos++;
            }
            else
            {
                I2C1->DATAR = 0xFFu;
            }
            return;
        }
        if (s_tx_pos < s_tx_len)
        {
            I2C1->DATAR = s_tx[s_tx_pos];
            s_tx_pos++;
        }
        return;
    }
    if ((sr1 & I2C_STAR1_RXNE) != 0u)
    {
        b = (uint8_t)I2C1->DATAR;
        if (s_slave != 0u)
        {
            if (s_got < I2C_IO_MAX)
            {
                s_rx[s_got] = b;
                s_got++;
            }
            return;
        }
        if (s_got < I2C_IO_MAX)
        {
            s_rx[s_got] = b;
        }
        s_got++;
        if ((uint8_t)(s_got + 1u) == s_need)
        {
            I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_ACK;
            i2c_stop_now();
        }
        if (s_got >= s_need)
        {
            s_done = 1u;
            s_busy = 0u;
        }
        return;
    }
    if ((sr1 & I2C_STAR1_STOPF) != 0u)
    {
        (void)I2C1->STAR1;
        I2C1->CTLR1 |= I2C_CTLR1_PE;
        if (s_slave != 0u && s_got != 0u)
        {
            i2c_queue_rep(0u, s_rx, s_got);
            s_got = 0u;
        }
        i2c_slave_pin();
    }
}

void I2C1_ER_IRQHandler(void)
{
    uint16_t sr1;

    sr1 = I2C1->STAR1;
    if ((sr1 & I2C_STAR1_AF) != 0u)
    {
        I2C1->STAR1 &= (uint16_t)~I2C_STAR1_AF;
        if (s_slave == 0u)
        {
            i2c_stop_now();
            s_nack = 1u;
            s_done = 1u;
            s_busy = 0u;
        }
        else
        {
            i2c_slave_pin();
        }
    }
    if ((sr1 & (I2C_STAR1_BERR | I2C_STAR1_ARLO | I2C_STAR1_OVR)) != 0u)
    {
        I2C1->STAR1 &= (uint16_t)~(I2C_STAR1_BERR | I2C_STAR1_ARLO | I2C_STAR1_OVR);
        if (s_slave == 0u)
        {
            i2c_stop_now();
            s_nack = 1u;
            s_done = 1u;
            s_busy = 0u;
        }
    }
}

static result_t i2c_speed(uint8_t fast)
{
    if (fast > 1u)
    {
        return RES_PARAM;
    }
    if (s_on == 0u)
    {
        return RES_STATE;
    }
    if (s_busy != 0u || s_scan != 0u)
    {
        return RES_BUSY;
    }
    s_fast = fast;
    i2c_apply_clock();
    return RES_OK;
}

const drv_i2c_driver_t drv_i2c =
{
    .pf_start = i2c_start,
    .pf_stop  = i2c_stop,
    .pf_role  = i2c_role,
    .pf_speed = i2c_speed,
    .pf_write = i2c_write,
    .pf_read  = i2c_read,
    .pf_scan  = i2c_scan,
    .pf_poll  = i2c_poll,
    .pf_take  = i2c_take,
};
