/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: 
 *       芯片自己做 256 字节快速擦除时，会不会把同一个 4KB 扇区里
 *       没被点名的其他页一起擦掉。
 *
 *
 * 地址（零等待区，物理地址从 0x08000000，4KB 一个扇区，256B 一页）
 *       扇区 0  0x08000000–0x08000FFF  Boot 2KB + A 面前 2KB
 *         0x08000000  本程序自己，前 2KB。擦除前后做异或，看它变没变。
 *         0x08000800  和程序同一扇区、紧挨着的下一页。这次只擦这一页。
 *         0x08000900  同一扇区更后面。这次故意不写，只读。整片擦过之后
 *                     它应该一直是空白值，不能拿它判断“擦除把邻居毁了”。
 *       扇区 15 0x0800F000–0x0800FFFF  B 面尾巴 + 标志区
 *         0x0800F000  先整页写成图案，然后不许再碰。
 *         0x0800F700  先整页写成图案，然后不许再碰。和 F800 页首尾相接。
 *         0x0800F800  只擦这一页。它离正在执行的程序很远，擦的时候
 *                     就算卡住，程序本体也还在。
 *
 * 做法
 *       1. 先给 F000、F700 各擦一页再写上图案（tag 按字或上序号 0..63）。
 *       2. 只擦 F800。再读 F000、F700：图案还在，就说明扇区 15 里
 *          256B 擦没有连带擦掉另外两页。
 *       3. 再擦 0x08000800。再给前 2KB 做一次异或：和擦之前相同，
 *          就说明扇区 0 里擦掉 A 面第一页，没有把 Boot 一起擦掉。
 *       4. 被擦的两页读回来必须是 0xE339E339。这是本芯片的擦除值，
 *          不是 0xFF。阶段 0.4 用同一次读数结案。
 *
 * 串口：USART1 TX = PA9，115200 8N1。系统时钟 96MHz，BRR = 833。
 *       每一步先把字母发出去再做下一步。停在哪个字母，就是卡在它后面
 *       那一步。结果区没有延时，会刷得很快，抓到完整的一组 R 到 Y 即可。
 *
 * 过程字母（只出现一次）
 *       1  串口已打开
 *       2  擦除前，前 2KB 的异或校验和
 *       3  解锁前的 FLASH->CTLR。本板实测 00008080，即 LOCK|FLOCK
 *       4  解锁前的 FLASH->STATR。本板实测 00000000
 *       5  即将解锁。若停在 5 之前，多半是在用户态写了 mstatus
 *       6  解锁之后的 CTLR
 *       7  开始擦并写 0xF000
 *       8  开始擦并写 0xF700
 *       9  0xF000 第一个字。写成功就是 A5A5F000
 *       A  开始只擦 0xF800
 *       C  擦 0xF800 已经返回。字母 B 没有使用，从 A 跳到 C 是正常的
 *       D  开始擦 0x08000800。这一页和本程序在同一个 4KB 扇区
 *       E  擦 0x08000800 已经返回，程序还在跑
 *       T  后面跟一个数：某次擦写等忙超时。数是当时的 STATR，最低位被置 1。
 *          打完这一行就停住，不会再刷 R 到 Y
 *       H  进了 HardFault。打完就停住
 *
 * 结果字母（死循环里反复刷。这里的 T 和上面的超时 T 不是一回事：
 *           超时 T 只打一次就停；结果 T 总是夹在 S 和 U 中间一起刷）
 *       R  判定。1 = 通过，0 = 不通过。上电初值是 0xEE，表示还没判完；
 *          循环里不会再出现 EE
 *       S  擦 0x0800 之前，前 2KB 异或和
 *       T  擦 0x0800 之后，前 2KB 异或和。改过本文件再编译，这个数会变。
 *          变了不算失败，S 和 T 相等才算前 2KB 没被破坏
 *       U  0x08000900 的第一个字。这次没有写过它，不参与通过判定。
 *          整片擦除之后读到 E339E339 只说明那里是空的
 *       V  0xF000 第一个字。整页 64 个字都要等于 A5A5F000 或上 0..63 才算过
 *       W  0xF700 第一个字。整页都要等于 A5A5F700 或上 0..63 才算过
 *       X  0x08000800 第一个字。刚被本程序擦掉，通过时是 E339E339
 *       Y  0xF800 第一个字。刚被本程序擦掉，通过时是 E339E339
 *
 * 通过条件（五项同时成立，R 才是 1）
 *       S == T
 *       F000 整页图案还在
 *       F700 整页图案还在
 *       X == 0xE339E339
 *       Y == 0xE339E339
 *
 *      本板实测，R = 1：
 *       S = T = 50823BCE
 *       U = E339E339
 *       V = A5A5F000
 *       W = A5A5F700
 *       X = Y = E339E339
 *       结论：256 字节快速擦除只擦被点名的那一页。扇区 0 里擦 0x0800
 *       没有改动前 2KB；扇区 15 里擦 0xF800 没有改动 F000 和 F700。
 *       擦完读回是 0xE339E339。2KB + 30KB + 30KB + 2KB 这套分法可以继续用，
 *       扇区 0 和扇区 15 允许被 256B 擦写从中切开。
 *
 * 这次没有证明的事
 *       WCH-Link 用它自己的默认擦除去下载一份链接在 0x0800 的程序时，
 *       会不会按 4KB 把 Boot 一起擦掉。这一半还没做。没做之前，
 *       Boot 和 App 联调仍然用一份合并 hex。
 *
 * 和这次结论有关、但不是这次代码直接测出来的事
 *       更早一版先擦 0x08000900，串口停在当时的 “7”，随后调试器报
 *       “设置芯片型号失败”。那次是擦除过程中内核停住，复位会再次走进
 *       同一段代码，所以复位救不回来；掉电全片擦除能恢复，是因为上电瞬间
 *       调试器可以先按住内核，这时它还没取 Flash 里的第一条指令。
 *       不能把那次卡死理解成“256B 擦会把同扇区全部擦掉”。本次跑完并读回，
 *       说明擦完以后邻居页还在。
 *
 * 写这套操作时踩过、注释里留着以免再踩的点
 *       只等 STATR 的 bit0（BSY）。bit1（WR_BSY）空闲时读出来是 1，
 *       连它一起等会永远等下去。
 *       已经解锁时再写两把密钥，会把控制器锁到下次复位，接着访问 Flash
 *       会总线错误。所以先看 LOCK、FLOCK，置位了才写 KEYR / MODEKEYR。
 *       启动文件用 mret 进入 main，main 在用户态。这里执行 csrci mstatus, 8
 *       会进异常，串口停在 5 之前。本测试不关中断。
 *       默认的 HardFault 是汇编里的死循环，不打印。本文件用同名 C 函数
 *       盖过它，进异常时打一个 H。
 *******************************************************************************/
#include "ch32v20x.h"

/* 标准解锁序列。先写 KEYR 清 LOCK（bit7），再写 MODEKEYR 清 FLOCK（bit15）。 */
#define S0_KEY1       0x45670123u
#define S0_KEY2       0xCDEF89ABu
#define S0_LOCK       0x00000080u
#define S0_FLOCK      0x00008000u
/* 快速页擦除：置 PAGE_ER，写 ADDR，再置 STRT。一页 256 字节。
 * 不用标准 4KB 擦，也不用 FLASH_ROM_ERASE（它会按更大的块擦）。 */
#define S0_PAGE_ER    0x00020000u
#define S0_STRT       0x00000040u
/* 快速页编程：置 PAGE_PG，连续写 64 个字，再置 PG_STRT 提交。 */
#define S0_PAGE_PG    0x00010000u
#define S0_PG_STRT    0x00200000u
/* 只看这一位。不要和 bit1 一起等。 */
#define S0_BSY        0x00000001u

#define S0_ADDR_BOOT  0x08000000u
#define S0_ADDR_900   0x08000900u
#define S0_ADDR_800   0x08000800u
#define S0_ADDR_F000  0x0800F000u
#define S0_ADDR_F700  0x0800F700u
#define S0_ADDR_F800  0x0800F800u
/* 一页 64 个字写成 tag|0、tag|1 … tag|63。tag 的低 8 位是 0，序号不会盖住它。
 * 所以这两页的第一个字读出来就是 tag 本身。 */
#define S0_TAG_900    0xA5A50900u
#define S0_TAG_F000   0xA5A5F000u
#define S0_TAG_F700   0xA5A5F700u

/* 全部放全局，串口打不出来时调试器还能直接看。
 * pass 初值 0xEE：还没跑到判定。0 不通过，1 通过。
 * e800、ef800 中途临时存过擦除的返回值，判定前会被读回的那个字盖掉。
 * 循环里打印的 X、Y 是盖掉之后的读数，不是擦除函数的返回值。 */
volatile uint32_t g_s0_pass = 0xEEu;
volatile uint32_t g_s0_sum0;
volatile uint32_t g_s0_sum1;
volatile uint32_t g_s0_p900;
volatile uint32_t g_s0_pf000;
volatile uint32_t g_s0_pf700;
volatile uint32_t g_s0_e800;
volatile uint32_t g_s0_ef800;

/* 发送完成（STATR bit6）之前不返回，避免下一步擦写时把这一个字母截掉。 */
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
 * CTLR1 = UE | TE。 */
static void uart_init(void)
{
    RCC->APB2PCENR |= RCC_IOPAEN | RCC_USART1EN;
    GPIOA->CFGHR &= ~(0xFu << 4);
    GPIOA->CFGHR |= (0xBu << 4);
    USART1->BRR = 833u;
    USART1->CTLR1 = 0x2008u;
}

/* 前 2KB 按字异或。不是 CRC，只用来回答“这 512 个字有没有任何一个变了”。 */
static uint32_t sum_boot(void)
{
    const uint32_t *p;
    uint32_t i;
    uint32_t sum;

    p = (const uint32_t *)S0_ADDR_BOOT;
    sum = 0u;
    for (i = 0u; i < (0x800u / 4u); i++)
    {
        sum ^= p[i];
    }
    return sum;
}

/* 返回 0 表示已经空闲。
 * 最多等 200 万次。超时返回 STATR 并且最低位置 1，这样返回值不会是 0，
 * 调用者可以把它当成失败原因从串口打出去，而不是在 BSY 上死等。 */
static uint32_t flash_wait(void)
{
    uint32_t n;

    for (n = 0u; n < 2000000u; n++)
    {
        if ((FLASH->STATR & S0_BSY) == 0u)
        {
            return 0u;
        }
    }
    return FLASH->STATR | 1u;
}

/* 只在对应锁位还是 1 的时候写密钥。已经解开再写，会锁到复位。 */
static void flash_unlock(void)
{
    if ((FLASH->CTLR & S0_LOCK) != 0u)
    {
        FLASH->KEYR = S0_KEY1;
        FLASH->KEYR = S0_KEY2;
    }
    if ((FLASH->CTLR & S0_FLOCK) != 0u)
    {
        FLASH->MODEKEYR = S0_KEY1;
        FLASH->MODEKEYR = S0_KEY2;
    }
}

/* 擦 addr 所在的 256 字节页，不动同扇区的其他页。
 * 返回 0 成功；非 0 是 flash_wait 的超时值。 */
static uint32_t flash_erase_page(uint32_t addr)
{
    uint32_t st;

    st = flash_wait();
    if (st != 0u)
    {
        return st;
    }
    FLASH->CTLR |= S0_PAGE_ER;
    FLASH->ADDR = addr;
    FLASH->CTLR |= S0_STRT;
    st = flash_wait();
    FLASH->CTLR &= ~S0_PAGE_ER;
    return st;
}

/* 把这一页写成 tag|0 .. tag|63。不等待失败，调用前要保证控制器已空闲。 */
static void flash_program_page(uint32_t addr, uint32_t tag)
{
    uint32_t i;

    flash_wait();
    FLASH->CTLR |= S0_PAGE_PG;
    flash_wait();
    for (i = 0u; i < 64u; i++)
    {
        *(volatile uint32_t *)(addr + (i * 4u)) = tag | i;
        flash_wait();
    }
    FLASH->CTLR |= S0_PG_STRT;
    flash_wait();
    FLASH->CTLR &= ~S0_PAGE_PG;
}

/* 整页 64 个字都对才返回 1。只看第一个字会把“写了一半”当成成功。 */
static uint32_t page_ok(uint32_t addr, uint32_t tag)
{
    uint32_t i;

    for (i = 0u; i < 64u; i++)
    {
        if (*(volatile uint32_t *)(addr + (i * 4u)) != (tag | i))
        {
            return 0u;
        }
    }
    return 1u;
}

/* 先擦再写。擦失败就把超时值交回去，不再写。 */
static uint32_t paint(uint32_t addr, uint32_t tag)
{
    uint32_t st;

    st = flash_erase_page(addr);
    if (st != 0u)
    {
        return st;
    }
    flash_program_page(addr, tag);
    return 0u;
}

/* 盖过启动文件里的弱符号。默认处理是跳到自己，串口上什么都没有。 */
void HardFault_Handler(void)
{
    tx_mark('H');
    while (1)
    {
    }
}

int main(void)
{
    uart_init();
    tx_mark('1');
    g_s0_sum0 = sum_boot();
    tx_val('2', g_s0_sum0);
    tx_val('3', FLASH->CTLR);
    tx_val('4', FLASH->STATR);

    /* main 在用户态，写 mstatus 会进异常。这里不关中断。 */
    tx_mark('5');
    flash_unlock();
    tx_val('6', FLASH->CTLR);

    /* 先写扇区 15 里离程序很远的两页。擦它们自己是为了能写进图案，
     * 真正要观察的擦除是后面那一次只擦 F800。 */
    tx_mark('7');
    g_s0_ef800 = paint(S0_ADDR_F000, S0_TAG_F000);
    if (g_s0_ef800 != 0u)
    {
        tx_val('T', g_s0_ef800);
        while (1)
        {
        }
    }
    tx_mark('8');
    g_s0_ef800 = paint(S0_ADDR_F700, S0_TAG_F700);
    if (g_s0_ef800 != 0u)
    {
        tx_val('T', g_s0_ef800);
        while (1)
        {
        }
    }
    tx_val('9', *(volatile uint32_t *)S0_ADDR_F000);
    tx_mark('A');
    g_s0_ef800 = flash_erase_page(S0_ADDR_F800);
    if (g_s0_ef800 != 0u)
    {
        tx_val('T', g_s0_ef800);
        while (1)
        {
        }
    }
    tx_mark('C');

    /* 这是离程序最近的一刀：同一扇区，下一页。
     * 若 256B 擦其实会擦整扇区，程序会在这一步消失，后面的 E 和 R 都不会出现。 */
    tx_mark('D');
    g_s0_e800 = flash_erase_page(S0_ADDR_800);
    tx_mark('E');
    flash_wait();
    FLASH->CTLR |= S0_FLOCK;
    FLASH->CTLR |= S0_LOCK;

    g_s0_sum1 = sum_boot();
    g_s0_p900 = *(volatile uint32_t *)S0_ADDR_900;
    g_s0_pf000 = *(volatile uint32_t *)S0_ADDR_F000;
    g_s0_pf700 = *(volatile uint32_t *)S0_ADDR_F700;
    /* 从这里起 e800、ef800 不再是擦除返回值，而是两页擦完后的第一个字。 */
    g_s0_e800 = *(volatile uint32_t *)S0_ADDR_800;
    g_s0_ef800 = *(volatile uint32_t *)S0_ADDR_F800;

    if ((g_s0_sum0 == g_s0_sum1) &&
        (page_ok(S0_ADDR_F000, S0_TAG_F000) != 0u) &&
        (page_ok(S0_ADDR_F700, S0_TAG_F700) != 0u) &&
        (g_s0_e800 == 0xE339E339u) &&
        (g_s0_ef800 == 0xE339E339u))
    {
        g_s0_pass = 1u;
    }
    else
    {
        g_s0_pass = 0u;
    }

    /* 无延时反复刷。一组的顺序固定是 R S T U V W X Y。 */
    while (1)
    {
        tx_val('R', g_s0_pass);
        tx_val('S', g_s0_sum0);
        tx_val('T', g_s0_sum1);
        tx_val('U', g_s0_p900);
        tx_val('V', g_s0_pf000);
        tx_val('W', g_s0_pf700);
        tx_val('X', g_s0_e800);
        tx_val('Y', g_s0_ef800);
    }
}
