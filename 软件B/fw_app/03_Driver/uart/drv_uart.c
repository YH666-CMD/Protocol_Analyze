#include "drv_uart.h"
#include "config.h"
#include "pin_map.h"
#include "bsp_clk.h"
#include "bsp_board.h"
#include "ch32v20x.h"

/*
 * 接收不走 DMA。实测 DMA 每帧只搬走 2 到 3 个字节就置位溢出，后面的字节外设不再交给 DMA。
 * 115200 下一字节约 87us，接收中断里读走数据寄存器来得及。
 * 发送也不在任务里死等。字节进队列后由发送空中断写寄存器，接收和发送同时走。
 * 启动文件里这个中断是 weak。强符号放在本文件，不改 User/ch32v20x_it.c。
 * 时基优先级更高，可以打断这里，但它不碰这两只队列。
 */
#define UART_TX_FIFO            128u
#define UART_TX_MASK            (UART_TX_FIFO - 1u)
#define UART_TXEIE              ((uint16_t)0x0080)   /* CTLR1 bit7，发送寄存器空中断 */
#define UART_IRQ_PREEMPT        1u
#define UART_IRQ_SUB            1u

#if ((UART_TX_FIFO & UART_TX_MASK) != 0u)
#error "UART_TX_FIFO must be a power of two"
#endif

static uint8_t s_rx_unit;           /* 9 位帧两个字节，其余一个字节 */
static uart_line_t s_line;
static uint8_t s_ready;
static rb_t s_rx;
static uint8_t s_tx[UART_TX_FIFO];
static volatile uint16_t s_tx_head; /* 下一个要写的位置，只由主循环改 */
static volatile uint16_t s_tx_tail; /* 下一个要发的位置，只由发送中断改 */

static const uart_line_t s_line_boot =
{
    .baud      = UART_BAUDRATE,
    .data_bits = UART_DATA_BITS_8,
    .stop_bits = UART_STOP_BITS_1,
    .parity    = UART_PARITY_NONE,
};

void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

static void uart_barrier(void)
{
    __asm__ volatile("" ::: "memory");
}

/* 先读过状态寄存器之后再读数据寄存器，空闲和溢出才会掉下去。 */
static void uart_idle_clear(void)
{
    (void)USART1->STATR;
    (void)USART1->DATAR;
}

/* 把刚读到的一帧推进环。9 位占两个字节，7 位去掉校验。 */
static void uart_rx_store_dr(uint16_t dr)
{
    uint8_t raw[2];
    uint16_t n;

    if (s_rx_unit == 2u)
    {
        raw[0] = (uint8_t)(dr & 0xFFu);
        raw[1] = (uint8_t)((dr >> 8) & 0xFFu);
        n = 2u;
    }
    else
    {
        raw[0] = (uint8_t)(dr & 0xFFu);
        if (s_line.data_bits == UART_DATA_BITS_7)
        {
            raw[0] &= 0x7Fu;
        }
        n = 1u;
    }
    (void)rb_sw_driver.pf_rb_write_buf(&s_rx, raw, n);
}

/* 队列里还有字节就写进发送寄存器。空了就关掉发送中断，避免空寄存器一直打断。 */
static void uart_tx_pump(void)
{
    uint16_t tail;

    if ((USART1->CTLR1 & UART_TXEIE) == 0u)
    {
        return;
    }
    if ((USART1->STATR & USART_FLAG_TXE) == 0u)
    {
        return;
    }

    tail = s_tx_tail;
    if (tail == s_tx_head)
    {
        USART1->CTLR1 &= (uint16_t)~UART_TXEIE;
        return;
    }

    USART1->DATAR = s_tx[tail];
    uart_barrier();
    s_tx_tail = (uint16_t)((tail + 1u) & UART_TX_MASK);
}

void USART1_IRQHandler(void)
{
    uint16_t sr;
    uint16_t dr;

    sr = USART1->STATR;
    if ((sr & (USART_FLAG_RXNE | USART_FLAG_ORE)) != 0u)
    {
        dr = USART1->DATAR;
        if ((sr & USART_FLAG_RXNE) != 0u)
        {
            uart_rx_store_dr(dr);
        }
    }

    uart_tx_pump();
}

static void uart_tx_flush(void)
{
    USART1->CTLR1 &= (uint16_t)~UART_TXEIE;
    s_tx_head = 0u;
    s_tx_tail = 0u;
}

static void uart_gpio_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
    else if (port == GPIOB)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    else if (port == GPIOC)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
    else if (port == GPIOD)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
}

static void uart_gpio_init(void)
{
    GPIO_InitTypeDef gpio;

    uart_gpio_clock(BOARD_UART_TX_PORT);
    uart_gpio_clock(BOARD_UART_RX_PORT);

    gpio.GPIO_Pin = BOARD_UART_TX_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(BOARD_UART_TX_PORT, &gpio);

    gpio.GPIO_Pin = BOARD_UART_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BOARD_UART_RX_PORT, &gpio);
}

static void uart_nvic_on(uint8_t channel)
{
    NVIC_InitTypeDef nvic;

    nvic.NVIC_IRQChannel = channel;
    nvic.NVIC_IRQChannelPreemptionPriority = UART_IRQ_PREEMPT;
    nvic.NVIC_IRQChannelSubPriority = UART_IRQ_SUB;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

/* 波特率要能写进 12 位整数分频。装不下就拒绝，不改寄存器。 */
static result_t uart_baud_ok(uint32_t baud)
{
    RCC_ClocksTypeDef clocks;
    uint32_t pclk;
    uint32_t divider;

    if (baud == 0u)
    {
        return RES_PARAM;
    }

    RCC_GetClocksFreq(&clocks);
    pclk = clocks.PCLK2_Frequency;
    if (pclk < 16u)
    {
        return RES_STATE;
    }
    if (baud > (pclk / 16u))
    {
        return RES_PARAM;
    }

    divider = (25u * pclk) / (4u * baud);
    if ((divider / 100u) == 0u || (divider / 100u) > 0x0FFFu)
    {
        return RES_PARAM;
    }
    return RES_OK;
}

/*
 * 把串口助手的四个参数换成芯片的字长、停止位、校验。
 * wide 输出 1 表示这一帧是 9 位，环里占两个字节。
 */
static result_t uart_line_map(const uart_line_t *line, USART_InitTypeDef *usart, uint8_t *wide)
{
    result_t rc;

    if (line == NULL || usart == NULL || wide == NULL)
    {
        return RES_PARAM;
    }

    switch (line->stop_bits)
    {
    case UART_STOP_BITS_1:
        usart->USART_StopBits = USART_StopBits_1;
        break;
    case UART_STOP_BITS_0_5:
        usart->USART_StopBits = USART_StopBits_0_5;
        break;
    case UART_STOP_BITS_2:
        usart->USART_StopBits = USART_StopBits_2;
        break;
    case UART_STOP_BITS_1_5:
        usart->USART_StopBits = USART_StopBits_1_5;
        break;
    default:
        return RES_PARAM;
    }

    switch (line->parity)
    {
    case UART_PARITY_NONE:
        usart->USART_Parity = USART_Parity_No;
        break;
    case UART_PARITY_EVEN:
        usart->USART_Parity = USART_Parity_Even;
        break;
    case UART_PARITY_ODD:
        usart->USART_Parity = USART_Parity_Odd;
        break;
    default:
        return RES_PARAM;
    }

    *wide = 0u;
    if (line->data_bits == UART_DATA_BITS_8 && line->parity == UART_PARITY_NONE)
    {
        usart->USART_WordLength = USART_WordLength_8b;
    }
    else if (line->data_bits == UART_DATA_BITS_8)
    {
        /* 8 位数据加 1 位校验，字长是 9。校验在第 9 位，环里只留低 8 位。 */
        usart->USART_WordLength = USART_WordLength_9b;
    }
    else if (line->data_bits == UART_DATA_BITS_7 && line->parity != UART_PARITY_NONE)
    {
        usart->USART_WordLength = USART_WordLength_8b;
    }
    else if (line->data_bits == UART_DATA_BITS_9 && line->parity == UART_PARITY_NONE)
    {
        usart->USART_WordLength = USART_WordLength_9b;
        *wide = 1u;
    }
    else
    {
        return RES_PARAM;
    }

    rc = uart_baud_ok(line->baud);
    if (rc != RES_OK)
    {
        return rc;
    }

    usart->USART_BaudRate = line->baud;
    usart->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart->USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    return RES_OK;
}

/* 关中断后重配。返回时新波特率和帧格式已经在跑。 */
static result_t uart_line_apply(const uart_line_t *line)
{
    USART_InitTypeDef usart;
    result_t rc;
    uint32_t irq;
    uint8_t wide;

    rc = uart_line_map(line, &usart, &wide);
    if (rc != RES_OK)
    {
        return rc;
    }

    irq = bsp_board_irq_lock();

    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);
    uart_tx_flush();
    /* 接收请求留在复位值。DMA 和接收中断不能一起读数据寄存器。 */
    USART1->CTLR3 &= (uint16_t)~USART_DMAReq_Rx;
    USART_Cmd(USART1, DISABLE);

    USART_Init(USART1, &usart);
    s_line = *line;
    s_rx_unit = (wide != 0u) ? 2u : 1u;

    USART_Cmd(USART1, ENABLE);
    uart_idle_clear();
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    bsp_board_irq_unlock(irq);
    return RES_OK;
}

static result_t uart_init(void)
{
    result_t rc;

    s_ready = 0u;
    rb_sw_driver.pf_rb_init(&s_rx);
    uart_tx_flush();
    uart_gpio_init();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    rc = uart_line_apply(&s_line_boot);
    if (rc != RES_OK)
    {
        return rc;
    }

    uart_nvic_on(USART1_IRQn);
    s_ready = 1u;
    return RES_OK;
}

static result_t uart_line_set(const uart_line_t *line)
{
    if (s_ready == 0u)
    {
        return RES_STATE;
    }
    return uart_line_apply(line);
}

static result_t uart_line_get(uart_line_t *line)
{
    if (line == NULL)
    {
        return RES_PARAM;
    }
    if (s_ready == 0u)
    {
        return RES_STATE;
    }
    *line = s_line;
    return RES_OK;
}

static rb_t *uart_rx_ring(void)
{
    return &s_rx;
}

/* 空位会变多不会变少：发送中断只取走字节。先看到有空位，紧接着入队就能放进去。 */
static uint16_t uart_tx_free(void)
{
    uint16_t used;

    used = (uint16_t)((s_tx_head - s_tx_tail) & UART_TX_MASK);
    return (uint16_t)(UART_TX_MASK - used);
}

static result_t uart_putc(uint8_t ch)
{
    uint32_t irq;
    uint16_t head;
    uint16_t next;

    if (s_ready == 0u)
    {
        return RES_STATE;
    }

    irq = bsp_board_irq_lock();
    head = s_tx_head;
    next = (uint16_t)((head + 1u) & UART_TX_MASK);
    if (next == s_tx_tail)
    {
        bsp_board_irq_unlock(irq);
        return RES_BUSY;
    }

    s_tx[head] = ch;
    uart_barrier();
    s_tx_head = next;
    USART1->CTLR1 |= UART_TXEIE;
    bsp_board_irq_unlock(irq);
    return RES_OK;
}

const drv_uart_driver_t drv_uart =
{
    .pf_init     = uart_init,
    .pf_rx_ring  = uart_rx_ring,
    .pf_putc     = uart_putc,
    .pf_tx_free  = uart_tx_free,
    .pf_line_set = uart_line_set,
    .pf_line_get = uart_line_get,
};
