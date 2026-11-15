/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: 只读，不解锁、不擦、不写 Flash。
 *
 * 上电就擦 0x08010000。擦除的忙位若不返回，CPU 从 Flash 取指会被挡住，
 * 调试器叫不停。按复位又会立刻再擦一次，所以下载完就认不出芯片。
 * 恢复要用 WCH-LinkUtility 的 Clear All Code Flash - By Power Off，
 * 按提示断电再上电，不要只按复位。
 *
 * 本程序只读六个地址的第一个字，用来确认非零等待区能读。
 *       0  0x08010000  慢速代码区开头
 *       1  0x08014000  参数/统计区开头
 *       2  0x08018000  故障快照区开头
 *       3  0x0801C000  日志区开头
 *       4  0x0802C000  出厂镜像区开头
 *       5  0x08037F00  整片最后一页
 * 空白字应是 0xE339E339。读到别的值只说明那里有旧数据，不是失败。
 * 若某一页读的时候死掉，最后一行停在该页的 A 上。
 *
 * 2026-11-15 本板实测：六处 D 都是 E339E339，1 一直在打。
 *       非零等待区在 96MHz 下可以读。这是掉电全片擦除之后的空值。
 
 *
 * 串口：USART1 TX = PA9，115200 8N1。一组打完空大约 1 秒再打。
 *       1  新程序已起来。看到 1 就说明没有再去擦 Flash
 *       A  下面 D 的地址
 *       D  该地址上的 32 位字
 *       H  读的过程中进了 HardFault，后面停住
 *******************************************************************************/
#include "ch32v20x.h"

#define S5_N 6u

static const uint32_t s_addr[S5_N] = {
    0x08010000u,
    0x08014000u,
    0x08018000u,
    0x0801C000u,
    0x0802C000u,
    0x08037F00u,
};

static void tx_byte(char c)
{
    while ((USART1->STATR & 0x0080u) == 0u)
    {
    }
    USART1->DATAR = (uint16_t)c;
}

static void tx_str(const char *s)
{
    while (*s != '\0')
    {
        tx_byte(*s);
        s++;
    }
}

static void tx_hex(uint32_t v)
{
    int32_t shift;

    for (shift = 28; shift >= 0; shift -= 4)
    {
        uint32_t n;

        n = (v >> (uint32_t)shift) & 0xFu;
        tx_byte((char)(n < 10u ? ('0' + n) : ('A' + (n - 10u))));
    }
}

static void tx_mark(char m)
{
    tx_byte(m);
    tx_str("\r\n");
    while ((USART1->STATR & 0x0040u) == 0u)
    {
    }
}

static void tx_val(char m, uint32_t v)
{
    tx_byte(m);
    tx_byte(' ');
    tx_hex(v);
    tx_str("\r\n");
    while ((USART1->STATR & 0x0040u) == 0u)
    {
    }
}

static void uart_init(void)
{
    RCC->APB2PCENR |= RCC_IOPAEN | RCC_USART1EN;
    GPIOA->CFGHR &= ~(0xFu << 4);
    GPIOA->CFGHR |= (0xBu << 4);
    USART1->BRR = 833u;
    USART1->CTLR1 = 0x2008u;
}

static void pause(void)
{
    volatile uint32_t n;

    for (n = 0u; n < 12000000u; n++)
    {
    }
}

void HardFault_Handler(void)
{
    tx_mark('H');
    while (1)
    {
    }
}

int main(void)
{
    uint32_t i;
    uint32_t n;
    uint32_t word;

    uart_init();
    n = S5_N;

    while (1)
    {
        tx_mark('1');
        for (i = 0u; i < n; i++)
        {
            tx_val('A', s_addr[i]);
            word = *(volatile uint32_t *)s_addr[i];
            tx_val('D', word);
        }
        pause();
    }
}
