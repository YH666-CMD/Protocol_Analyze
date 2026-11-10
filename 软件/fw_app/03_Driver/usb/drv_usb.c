/*
 * drv_usb.c
 *
 *  Created on: 2026
 *      Author: YinHao
 *      introduction: USB 时钟、引脚、中断，以及对原厂库的一层封装。
 *
 * 原厂 USB 内核在本目录的 vendor 里，原样引用，不改写成这里的风格。
 * 全速 USB 的内核时钟是 48 MHz。分频由 USB_HCLK_HZ 决定，这里不改系统时钟。
 * 启动文件里 USB 中断是 weak。强符号放在本文件，不改 User/ch32v20x_it.c。
 *
 * 分组缓冲溢出位不在中断屏蔽字里。置上以后要软件写 0 才清，所以入口处清掉。
 * 打印函数整段放在文件末尾，USB_LOG_ENABLE 为 0 时编译器拿掉。
 */
#include "drv_usb.h"
#include "usb_log.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "ch32v20x.h"

#if (USB_HCLK_HZ == 144000000u)
#define USB_CLK_DIV                 RCC_USBCLKSource_PLLCLK_Div3
#elif (USB_HCLK_HZ == 96000000u)
#define USB_CLK_DIV                 RCC_USBCLKSource_PLLCLK_Div2
#else
#define USB_CLK_DIV                 RCC_USBCLKSource_PLLCLK_Div1
#endif

static uint8_t s_ready;

void USBWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USBWakeUp_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line18);
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    uint16_t istr;

    istr = _GetISTR();
    if ((istr & ISTR_DOVR) != 0u)
    {
        _SetISTR((uint16_t)CLR_DOVR);
        usb_log_pma_ovr();
    }
#if USB_LOG_ENABLE
    {
        uint16_t ep0_in;

        /* SETUP 位会被原厂处理清掉，所以在调用之前先读。 */
        ep0_in = 0u;
        if (((istr & ISTR_CTR) != 0u) && ((istr & 0x0Fu) == 0u))
        {
            ep0_in = _GetENDPOINT(ENDP0);
        }
#endif
        USB_Istr();
#if USB_LOG_ENABLE
        if ((istr & ISTR_ERR) != 0u)
        {
            usb_log_err(istr);
        }
        if ((istr & ISTR_RESET) != 0u)
        {
            usb_log_reset();
        }
        if ((ep0_in & EP_SETUP) != 0u)
        {
            usb_log_setup(Device_Info.USBbmRequestType, Device_Info.USBbRequest,
                          Device_Info.USBwValue, Device_Info.USBwLength);
        }
    }
#endif
}

/* 关掉上拉时主机认为设备拔出。再打开上拉，主机重新枚举。PA11、PA12 交给 USB 模块。 */
void USB_Port_Set(FunctionalState NewState, FunctionalState Pin_In_IPU)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    if (NewState)
    {
        _SetCNTR(_GetCNTR() & (uint16_t)(~(1u << 1)));
        GPIOA->CFGHR &= USB_GPIO_CFGHR_CLR;
        GPIOA->OUTDR &= (uint16_t)(~(3u << 11));
        GPIOA->CFGHR |= USB_GPIO_CFGHR_IN;
    }
    else
    {
        _SetCNTR(_GetCNTR() | (uint16_t)(1u << 1));
        GPIOA->CFGHR &= USB_GPIO_CFGHR_CLR;
        GPIOA->OUTDR &= (uint16_t)(~(3u << 11));
        GPIOA->CFGHR |= USB_GPIO_CFGHR_OUT;
    }

    if (Pin_In_IPU)
    {
        EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
    }
    else
    {
        EXTEN->EXTEN_CTR &= (uint32_t)(~EXTEN_USBD_PU_EN);
    }
}

void Enter_LowPowerMode(void)
{
    bDeviceState = SUSPENDED;
}

void Leave_LowPowerMode(void)
{
    if (Device_Info.Current_Configuration != 0u)
    {
        bDeviceState = CONFIGURED;
    }
    else
    {
        bDeviceState = ATTACHED;
    }
}

/* 按 usb_cfg.h 里的主频选分频，并打开 USBD 的总线时钟。不读时钟树，避免把整段时钟计算链进来。 */
static void usb_clock_on(void)
{
    RCC_USBCLKConfig(USB_CLK_DIV);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/* 打开唤醒线和 USB 低优先级中断。优先级数字来自 usb_cfg.h。 */
static void usb_irq_on(void)
{
    NVIC_InitTypeDef nvic;
    EXTI_InitTypeDef exti;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    EXTI_ClearITPendingBit(EXTI_Line18);
    exti.EXTI_Line = EXTI_Line18;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    nvic.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = USB_LP_PREEMPT;
    nvic.NVIC_IRQChannelSubPriority = USB_LP_SUB;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = USBWakeUp_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = USB_WKUP_PREEMPT;
    nvic.NVIC_IRQChannelSubPriority = USB_WKUP_SUB;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

static result_t usb_init(void)
{
    if (s_ready != 0u)
    {
        return RES_OK;
    }
    usb_clock_on();
    /* 先开中断再拉上拉。主机一看见设备就会发复位，中断没开就会把这次复位丢掉。 */
    usb_irq_on();
    __enable_irq();
    USB_Init();
    s_ready = 1u;
    return RES_OK;
}

static usb_state_t usb_state(void)
{
    return (usb_state_t)bDeviceState;
}

static result_t usb_line_get(usb_cdc_line_t *line)
{
    uint8_t raw[7];

    if (line == NULL)
    {
        return RES_PARAM;
    }
    if (s_ready == 0u)
    {
        return RES_STATE;
    }
    usb_cdc_line_get(raw);
    line->baud = (uint32_t)raw[0]
               | ((uint32_t)raw[1] << 8)
               | ((uint32_t)raw[2] << 16)
               | ((uint32_t)raw[3] << 24);
    line->stop_bits = raw[4];
    line->parity = raw[5];
    line->data_bits = raw[6];
    return RES_OK;
}

static uint16_t usb_tx_ready(void)
{
    if (s_ready == 0u || bDeviceState != CONFIGURED)
    {
        return 0u;
    }
    return usb_ep3_room();
}

static result_t usb_tx(const uint8_t *data, uint16_t len)
{
    uint8_t ok;

    if (data == NULL || len == 0u || len > USB_EP_MPS)
    {
        usb_log_tx(len, 0u);
        return RES_PARAM;
    }
    if (s_ready == 0u || bDeviceState != CONFIGURED)
    {
        usb_log_tx(len, 0u);
        return RES_STATE;
    }
    ok = (uint8_t)(USBD_ENDPx_DataUp(ENDP3, (uint8_t *)data, len) == USB_SUCCESS);
    usb_log_tx(len, ok);
    if (ok == 0u)
    {
        return RES_BUSY;
    }
    return RES_OK;
}

static result_t usb_on_rx_set(usb_rx_fn fn)
{
    usb_rx_hook_set(fn);
    return RES_OK;
}

const drv_usb_driver_t drv_usb =
{
    .pf_init      = usb_init,
    .pf_state     = usb_state,
    .pf_line_get  = usb_line_get,
    .pf_tx        = usb_tx,
    .pf_tx_ready  = usb_tx_ready,
    .pf_on_rx_set = usb_on_rx_set,
};

#if USB_LOG_ENABLE

static usb_log_fn s_log;
static uint16_t s_susp_n;

void usb_log_bind(usb_log_fn fn)
{
    s_log = fn;
}

void usb_log_attach(void)
{
    if (s_log != NULL)
    {
        (void)s_log("usb attach\r\n");
    }
}

void usb_log_reset(void)
{
    if (s_log == NULL)
    {
        s_susp_n = 0u;
        return;
    }
    if (s_susp_n > 1u)
    {
        (void)s_log("usb susp n=%u\r\n", (unsigned)s_susp_n);
    }
    s_susp_n = 0u;
    (void)s_log("usb reset cntr=%04x ep0=%04x fnr=%04x\r\n",
                (unsigned)_GetCNTR(), (unsigned)_GetENDPOINT(ENDP0), (unsigned)_GetFNR());
}

void usb_log_setup(uint8_t type, uint8_t req, uint16_t value, uint16_t length)
{
    if (s_log != NULL)
    {
        (void)s_log("usb setup t=%02x r=%02x v=%04x l=%04x\r\n",
                    (unsigned)type, (unsigned)req, (unsigned)value, (unsigned)length);
    }
}

void usb_log_address(uint8_t addr)
{
    if (s_log != NULL)
    {
        (void)s_log("usb addr=%u\r\n", (unsigned)addr);
    }
}

void usb_log_configured(uint8_t cfg)
{
    if (s_log != NULL)
    {
        (void)s_log("usb cfg=%u\r\n", (unsigned)cfg);
    }
}

void usb_log_pma_ovr(void)
{
    uint16_t addr;
    uint16_t d;
    volatile uint16_t *rx;

    if (s_log == NULL)
    {
        return;
    }
    addr = *(volatile uint16_t *)(PMAAddr + ((uint32_t)(_GetBTABLE() + 4u) * 2u));
    rx = (volatile uint16_t *)(PMAAddr + ((uint32_t)addr * 2u));
    d = rx[0];
    (void)s_log("usb pma ovr rc=%04x d=%04x ep=%04x\r\n",
                (unsigned)(*(volatile uint16_t *)(PMAAddr + ((uint32_t)(_GetBTABLE() + 6u) * 2u))),
                (unsigned)d, (unsigned)_GetENDPOINT(ENDP0));
}

void usb_log_err(uint16_t istr)
{
    if (s_log != NULL)
    {
        (void)s_log("usb err istr=%04x\r\n", (unsigned)istr);
    }
}

void usb_log_resume_blocked(void)
{
    if (s_susp_n < 0xFFFFu)
    {
        s_susp_n++;
    }
    if (s_susp_n == 1u && s_log != NULL)
    {
        (void)s_log("usb susp blocked\r\n");
    }
}

void usb_log_esof_bail(void)
{
    if (s_log != NULL)
    {
        (void)s_log("usb esof bail\r\n");
    }
}

void usb_log_line(const uint8_t raw[7])
{
    uint32_t baud;

    if (s_log == NULL || raw == NULL)
    {
        return;
    }
    baud = (uint32_t)raw[0]
         | ((uint32_t)raw[1] << 8)
         | ((uint32_t)raw[2] << 16)
         | ((uint32_t)raw[3] << 24);
    (void)s_log("usb line baud=%u stop=%u par=%u bits=%u\r\n",
                (unsigned)baud, (unsigned)raw[4], (unsigned)raw[5], (unsigned)raw[6]);
}

void usb_log_rx(uint16_t len)
{
    if (s_log != NULL)
    {
        (void)s_log("usb rx %u\r\n", (unsigned)len);
    }
}

void usb_log_tx(uint16_t len, uint8_t ok)
{
    if (s_log != NULL)
    {
        (void)s_log("usb tx %u ok=%u\r\n", (unsigned)len, (unsigned)ok);
    }
}

#endif
