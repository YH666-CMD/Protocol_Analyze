/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: 主程序入口
 * 系统时钟: 目标 144MHz（HSE 优先，失败则 HSI 回退，见 system_ch32v20x.c）
 *******************************************************************************/

#include "debug.h"

/* 芯片 96 位唯一 ID 的只读存储地址：低、中、高各 32 位 */
#define UID_BASE    ((uint32_t)0x1FFFF7E8)

/* 上电后写入。下标 0 为低 32 位，1 为中 32 位，2 为高 32 位 */
uint32_t g_mcu_uid[3];

static void Read_MCU_UID(void)
{
    g_mcu_uid[0] = *(volatile uint32_t *)(UID_BASE);
    g_mcu_uid[1] = *(volatile uint32_t *)(UID_BASE + 4);
    g_mcu_uid[2] = *(volatile uint32_t *)(UID_BASE + 8);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    Read_MCU_UID();

    while(1)
    {
    }
}
