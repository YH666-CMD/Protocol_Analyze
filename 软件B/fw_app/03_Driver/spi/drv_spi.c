#include "drv_spi.h"
#include "pin_map.h"
#include "ch32v20x.h"

/*
 * 只写本机要用的寄存器，不调用 SPI_Init，避免把整份 SPI 库链进来。
 * 8 位，MSB 先发，软件 NSS。时钟分频 64，和官方全双工例程一样。
 *
 * 模式号和时钟沿。切换时整段重写 CTLR1。
 *   0  CPOL 低，第 1 个边沿
 *   1  CPOL 低，第 2 个边沿
 *   2  CPOL 高，第 1 个边沿
 *   3  CPOL 高，第 2 个边沿
 */
#define SPI_BR_64      (SPI_CTLR1_BR_0 | SPI_CTLR1_BR_2)
#define SPI_SPIN_MAX   256u

static uint8_t s_on;
static uint8_t s_mode;
static uint8_t s_ring_ready;
static rb_t s_rx;

static const uint16_t s_mode_bit[4] =
{
    0u,
    SPI_CTLR1_CPHA,
    SPI_CTLR1_CPOL,
    (uint16_t)(SPI_CTLR1_CPOL | SPI_CTLR1_CPHA),
};

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

static void spi_pins_master(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);
    spi_nss_release();

    gpio.GPIO_Pin = BOARD_SPI_SCK_PIN | BOARD_SPI_MOSI_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(BOARD_SPI_SCK_PORT, &gpio);

    gpio.GPIO_Pin = BOARD_SPI_MISO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BOARD_SPI_MISO_PORT, &gpio);
}

static void spi_pins_quiet(void)
{
    GPIO_InitTypeDef gpio;

    spi_nss_release();
    gpio.GPIO_Pin = BOARD_SPI_SCK_PIN | BOARD_SPI_MOSI_PIN | BOARD_SPI_MISO_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BOARD_SPI_SCK_PORT, &gpio);
}

static void spi_hw_init(uint8_t mode)
{
    SPI1->CTLR1 &= (uint16_t)~SPI_CTLR1_SPE;
    SPI1->CTLR1 = (uint16_t)(SPI_CTLR1_MSTR | SPI_CTLR1_SSI | SPI_CTLR1_SSM |
                             SPI_BR_64 | s_mode_bit[mode]);
    SPI1->CTLR2 = 0u;
    SPI1->I2SCFGR = 0u;
    SPI1->CRCR = 7u;
    SPI1->CTLR1 |= SPI_CTLR1_SPE;
    (void)SPI1->DATAR;
    (void)SPI1->STATR;
    s_mode = mode;
}

static uint8_t spi_wait(uint16_t mask, uint8_t want_set)
{
    uint16_t n;

    for (n = 0u; n < SPI_SPIN_MAX; n++)
    {
        if (((SPI1->STATR & mask) != 0u) == (want_set != 0u))
        {
            return 1u;
        }
    }
    return 0u;
}

static result_t spi_start(void)
{
    if (s_on != 0u)
    {
        return RES_OK;
    }
    if (s_ring_ready == 0u)
    {
        rb_sw_driver.pf_rb_init(&s_rx);
        s_ring_ready = 1u;
    }
    spi_pins_master();
    spi_hw_init(0u);
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
    spi_pins_quiet();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
    s_on = 0u;
}

static rb_t *spi_rx_ring(void)
{
    if (s_ring_ready == 0u)
    {
        return NULL;
    }
    return &s_rx;
}

static result_t spi_mode(uint8_t mode)
{
    if (s_on == 0u)
    {
        return RES_STATE;
    }
    if (mode > 3u)
    {
        return RES_PARAM;
    }
    spi_hw_init(mode);
    return RES_OK;
}

static result_t spi_xfer(const uint8_t *tx, uint16_t len)
{
    uint16_t i;
    uint8_t dropped;
    uint8_t done;

    if (s_on == 0u)
    {
        return RES_STATE;
    }
    if (tx == NULL || len == 0u || len > SPI_XFER_MAX)
    {
        return RES_PARAM;
    }

    dropped = 0u;
    GPIO_ResetBits(BOARD_SPI_NSS_PORT, BOARD_SPI_NSS_PIN);
    for (i = 0u; i < len; i++)
    {
        if (spi_wait(SPI_STATR_TXE, 1u) == 0u)
        {
            break;
        }
        SPI1->DATAR = tx[i];
        if (spi_wait(SPI_STATR_RXNE, 1u) == 0u)
        {
            break;
        }
        if (!rb_sw_driver.pf_rb_write(&s_rx, (uint8_t)SPI1->DATAR))
        {
            dropped = 1u;
        }
    }
    done = spi_wait(SPI_STATR_BSY, 0u);
    GPIO_SetBits(BOARD_SPI_NSS_PORT, BOARD_SPI_NSS_PIN);
    if (i != len || done == 0u)
    {
        (void)SPI1->DATAR;
        (void)SPI1->STATR;
        return RES_OVERRUN;
    }
    if (dropped != 0u)
    {
        return RES_NO_MEM;
    }
    return RES_OK;
}

const drv_spi_driver_t drv_spi =
{
    .pf_start   = spi_start,
    .pf_stop    = spi_stop,
    .pf_rx_ring = spi_rx_ring,
    .pf_mode    = spi_mode,
    .pf_xfer    = spi_xfer,
};
