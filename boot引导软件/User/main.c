/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: 确认启动文件执行 mret 之后，main 处于用户态还是机器态。
 *       不擦、不写 Flash。复位、重新下载都是安全的。
 *
 * 为什么要测
 *       启动文件把 mstatus 写成 0x88（MPP = 00，MPIE = 1，MIE = 1），
 *       再把 main 的地址写入 mepc，然后 mret。
 *       按 RISC-V 的规定，MPP = 00 时 mret 会进入用户态。
 *       用户态不能写 mtvec、mstatus，也不能直接改特权级。
 *       所以 Boot 以后不能在 main 里直接跳到 App，要先进软件中断
 *       （中断服务函数在机器态），再在那里 jr。
 *       本测试就是把“main 到底在哪一态”读出来。
 *
 * 做法
 *       main 里执行 csrr mstatus，只读，不改。
 *       机器态：这条指令能执行完，串口打出 M 和读到的值。
 *       用户态：读机器态 CSR 是非法指令，会进异常。异常服务函数
 *       运行在机器态，在里面再读 mstatus、mcause、mepc。
 *       此时 mstatus 的 MPP（bit 12:11）保存的是进异常之前的特权级，
 *       也就是 main 的特权级。
 *
 * 串口：USART1 TX = PA9，115200 8N1。系统时钟仍是 96MHz，BRR = 833。
 *       先打一次过程字母，再反复打结果，每组之间空大约 1 秒。
 *
 * 过程字母（只出现一次）
 *       1  串口已打开
 *       2  下一条就是 csrr mstatus
 *
 * 结果，机器态（csrr 执行完了）
 *       M  main 里读到的 mstatus 原值
 *       P  其中的 MPP，bit 12:11。00 用户，11 机器。
 *          能打出 M，说明当前就是机器态；P 只是 mstatus 里留下的字段
 *
 * 结果，用户态（csrr 没执行完，进了异常）
 *       H  进的是 HardFault_Handler。本芯片的非法指令通常走这里
 *       N  进的是 NMI_Handler。若实际走到这个向量，也会把现场打出来
 *       C  mcause 原值。RISC-V 里非法指令是 2，最高位为 1 表示中断
 *       S  异常里读到的 mstatus 原值
 *       P  S 的 MPP（bit 12:11），即进异常之前的特权级。00 就是用户态
 *       E  mepc，应指向那条 csrr
 *
 * 怎么判
 *       看到 1、2 之后是 H（或 N），并且 P 为 00000000：main 在用户态。
 *       跳转继续用软件中断。
 *       看到 1、2 之后是 M：main 在机器态，和启动文件的写法不一致。
 *       只有 1、没有 2：异常发生在读 CSR 之前，这次不能用来判断特权级。
 *
 * 2026-11-15 本板实测：1、2 之后是 H。
 *       C = 00000002  非法指令
 *       S = 00000088  仍是启动文件写入的值。bit3、bit7 为 1，MPP 两位为 0。
 *                     进异常后 MIE 没有被清掉。判特权级不看整个 S，看 P
 *       P = 00000000  进异常前是用户态
 *       E = 000002D4  反汇编该地址就是 csrr s1, mstatus
 *       结论：main 在用户态。Boot 跳 App 必须在软件中断里做。
 *
 * 链接仍是 2KB。本文件不再做擦除测试，0.1 的实测结论在规划文档里。
 *******************************************************************************/
#include "ch32v20x.h"

/* 0 还没判完，1 main 读到了 mstatus，2 进了 NMI，3 进了 HardFault。 */
volatile uint32_t g_s3_from;
volatile uint32_t g_s3_mstatus;
volatile uint32_t g_s3_mcause;
volatile uint32_t g_s3_mepc;

/* 发送完成（STATR bit6）之前不返回，避免下一行把这一行截断。 */
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

/* 固定 8 位十六进制，不加 0x。 */
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

/* PA9 复用推挽 50MHz。CFGHR 的 pin9 半字节是 bit[7:4]，写成 0xB。
 * CTLR1 = UE | TE。96MHz 下 BRR = 833，对应 115200。 */
static void uart_init(void)
{
    RCC->APB2PCENR |= RCC_IOPAEN | RCC_USART1EN;
    GPIOA->CFGHR &= ~(0xFu << 4);
    GPIOA->CFGHR |= (0xBu << 4);
    USART1->BRR = 833u;
    USART1->CTLR1 = 0x2008u;
}

/* 约 1 秒。空循环用 volatile，避免被优化掉。只为了让串口助手能看清一组。 */
static void pause(void)
{
    volatile uint32_t n;

    for (n = 0u; n < 12000000u; n++)
    {
    }
}

/* CSR 地址是立即数，不能写成函数参数。三条分开读。 */
static uint32_t csr_mstatus(void)
{
    uint32_t v;

    __asm volatile ("csrr %0, mstatus" : "=r"(v));
    return v;
}

static uint32_t csr_mcause(void)
{
    uint32_t v;

    __asm volatile ("csrr %0, mcause" : "=r"(v));
    return v;
}

static uint32_t csr_mepc(void)
{
    uint32_t v;

    __asm volatile ("csrr %0, mepc" : "=r"(v));
    return v;
}

/* 异常里调用。这里已经是机器态，读 mstatus 不会再进一次异常。
 * MPP 取的是 bit 12:11，表示进异常之前的特权级。 */
static void report_trap(char which, uint32_t from)
{
    uint32_t st;
    uint32_t cause;
    uint32_t epc;
    uint32_t mpp;

    st = csr_mstatus();
    cause = csr_mcause();
    epc = csr_mepc();
    mpp = (st >> 11) & 3u;
    g_s3_from = from;
    g_s3_mstatus = st;
    g_s3_mcause = cause;
    g_s3_mepc = epc;

    while (1)
    {
        tx_mark(which);
        tx_val('C', cause);
        tx_val('S', st);
        tx_val('P', mpp);
        tx_val('E', epc);
        pause();
    }
}

/* 盖过启动文件里的弱符号。默认处理是跳到自己，串口上什么都没有。 */
void NMI_Handler(void)
{
    report_trap('N', 2u);
}

void HardFault_Handler(void)
{
    report_trap('H', 3u);
}

int main(void)
{
    uint32_t st;
    uint32_t mpp;

    uart_init();
    tx_mark('1');
    tx_mark('2');

    /* 用户态执行到这里会进异常，下面三行不会执行。 */
    st = csr_mstatus();
    mpp = (st >> 11) & 3u;
    g_s3_from = 1u;
    g_s3_mstatus = st;
    g_s3_mcause = 0u;
    g_s3_mepc = 0u;

    while (1)
    {
        tx_val('M', st);
        tx_val('P', mpp);
        pause();
    }
}
