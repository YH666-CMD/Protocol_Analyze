/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: 主程序 — 串口打印、读取 UID、PA0 每秒翻转
 *
 * 硬件连接:
 *   PA9  -> USART1 发送，115200，接 USB-TTL 的 RX
 *   PA0  -> LED 或测试引脚，每 1 秒翻转一次
 * 系统时钟: 目标 144MHz（HSE 优先，失败则 HSI 回退，见 system_ch32v20x.c）
 *******************************************************************************/

#include "debug.h"

/* 芯片 96 位唯一 ID 的只读存储地址 */
#define UID_BASE    ((uint32_t)0x1FFFF7E8)
#define UID_0       (UID_BASE)          /* 低 32 位 */
#define UID_1       (UID_BASE + 4)      /* 中 32 位 */
#define UID_2       (UID_BASE + 8)      /* 高 32 位 */

/* 读取并打印 96 位芯片唯一 ID（每颗芯片不同） */
static void Read_MCU_UID(void)
{
    uint32_t uid0 = *(volatile uint32_t *)UID_0;
    uint32_t uid1 = *(volatile uint32_t *)UID_1;
    uint32_t uid2 = *(volatile uint32_t *)UID_2;

    printf("MCU 96-bit UID: 0x%08X %08X %08X\r\n", uid2, uid1, uid0);
}

/* 打印当前时钟源，便于确认 PLL 是否生效、HSE 是否起振 */
static void Print_ClockSource(void)
{
    uint32_t sws = RCC->CFGR0 & RCC_SWS;
    const char *src;

    if(sws == RCC_SWS_HSI)
        src = "HSI";
    else if(sws == RCC_SWS_HSE)
        src = "HSE";
    else if(sws == RCC_SWS_PLL)
        src = "PLL";
    else
        src = "Unknown";

    printf("ClockSrc:%s HSE_RDY:%d PLL_RDY:%d\r\n",
           src,
           (RCC->CTLR & RCC_HSERDY) ? 1 : 0,
           (RCC->CTLR & RCC_PLLRDY) ? 1 : 0);
}

/* 初始化 PA0 为推挽输出，默认低电平 */
static void GPIO_PA0_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}

int main(void)
{
    /* 基础初始化：中断分组、时钟变量、延时、串口、GPIO */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    GPIO_PA0_Init();

    /* 上电信息，串口助手可查看 */
    printf("SystemClk:%d\r\n", SystemCoreClock);
    Print_ClockSource();
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());   /* CH32V203C8T6 型号标识 */
    Read_MCU_UID();
    printf("PA0 toggle every 1s\r\n");

    while(1)
    {
        /* 翻转 PA0 电平，延时 1 秒 */
        GPIO_WriteBit(GPIOA, GPIO_Pin_0,
            (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_0)));
        Delay_Ms(1000);
    }
}
