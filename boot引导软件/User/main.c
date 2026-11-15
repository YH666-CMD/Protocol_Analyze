/********************************** (C) COPYRIGHT *******************************
 * 文件: main.c
 * 说明: 上电只读，不碰 Flash 控制器。
 *       串口收到字母 E 之后，只对 0x08014000 做一次 256B 擦和整页写。
 *       这一页在参数区，不是慢速代码开头，也不是 32KB 块的起点。
 *
 * 为什么不在上电时擦
 *       擦除没完成时，CPU 从 Flash 取指会被挡住，调试器叫不停。
 *       复位会再擦一次。所以擦除必须由你发一个字母才开始，
 *       并且等忙的循环放在 RAM 里：忙位不返回时 CPU 还在跑，调试器能连上。
 *
 * 六处只读（2026-11-15 已读到全是 E339E339）
 *       0x08010000  0x08014000  0x08018000
 *       0x0801C000  0x0802C000  0x08037F00
 *
 * 这一页写入：64 个字为 tag|0 .. tag|63，tag = A5014000。
 *       只用 PAGE_ER / PAGE_PG。不用 4KB 擦，不用 32KB 块擦。
 *       做完不擦掉图案。
 *
 * 2026-11-15 本板实测，发 E 之后 R = 1：
 *       X = E339E339   擦完第一个字，整页 B = 1
 *       V = A5014000   写完第一个字，整页 K = 1
 *       0x08010000、0x08018000、0x0801C000、0x0802C000、0x08037F00
 *       仍是 E339E339，没有被这页擦除带走。
 *       结论：96MHz 下，非零等待区可以 256B 擦、整页写、再读回。
 *       等忙放在 RAM 里，是为了忙位不返回时调试器还能连上。
 *       其余五页不再单独测，按这一页的结果对待。取指不在本程序里测。
 *       没收到 E 之前反复打：
 *       1  程序在跑，还没擦
 *       A  地址
 *       D  该地址第一个字
 *       W  在等字母 E
 *       收到 E 之后多打：
 *       G  开始擦 0x08014000。停在 G 且调试器仍能连上，就是忙位没返回
 *       X  擦完后的第一个字，通过应为 E339E339
 *       B  1 = 擦完 64 字全是 E339E339
 *       V  写完后的第一个字，通过应为 A5014000
 *       K  1 = 写完 64 字全是 tag|序号
 *       T  等忙超时，但忙位已经掉了，控制器没锁死。数值是当时的 STATR
 *       R  1 = B 和 K 都是 1
 *       H  HardFault
 *******************************************************************************/
#include "ch32v20x.h"

#define S5_KEY1     0x45670123u
#define S5_KEY2     0xCDEF89ABu
#define S5_LOCK     0x00000080u
#define S5_FLOCK    0x00008000u
#define S5_PAGE_ER  0x00020000u
#define S5_STRT     0x00000040u
#define S5_PAGE_PG  0x00010000u
#define S5_PG_STRT  0x00200000u
#define S5_BSY      0x00000001u
#define S5_WR_BSY   0x00000002u
#define S5_ERASED   0xE339E339u
#define S5_PAGE     0x08014000u
#define S5_TAG      0xA5014000u
#define S5_N        6u
#define S5_SPINS    20000000u

static const uint32_t s_addr[S5_N] = {
    0x08010000u,
    0x08014000u,
    0x08018000u,
    0x0801C000u,
    0x0802C000u,
    0x08037F00u,
};

volatile uint32_t g_s5_done;
volatile uint32_t g_s5_x;
volatile uint32_t g_s5_b;
volatile uint32_t g_s5_v;
volatile uint32_t g_s5_k;
volatile uint32_t g_s5_r;
volatile uint32_t g_s5_t;

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

/* 整段在 RAM。忙位置住时不能再从 Flash 取指。
 * 参数都在寄存器里，循环里不读 Flash 上的常数。
 * 返回 0：要等的位已经清掉。
 * 返回非 0：超时，但 bit0 已经是 0，调用者还可以访问 Flash。
 * bit0 仍为 1 就停在本函数里，不再返回。 */
static uint32_t ram_wait(volatile uint32_t *statr, uint32_t mask, uint32_t spins)
    __attribute__((noinline, section(".data")));

static uint32_t ram_wait(volatile uint32_t *statr, uint32_t mask, uint32_t spins)
{
    uint32_t n;
    uint32_t st;

    for (n = 0u; n < spins; n++)
    {
        st = *statr;
        if ((st & mask) == 0u)
        {
            return 0u;
        }
    }
    st = *statr;
    if ((st & S5_BSY) != 0u)
    {
        while (1)
        {
        }
    }
    return st | 1u;
}

static void uart_init(void)
{
    RCC->APB2PCENR |= RCC_IOPAEN | RCC_USART1EN;
    GPIOA->CFGHR &= ~(0xFu << 4);
    GPIOA->CFGHR |= (0xBu << 4);
    GPIOA->CFGHR &= ~(0xFu << 8);
    GPIOA->CFGHR |= (0x8u << 8);
    GPIOA->BSHR = (1u << 10);
    USART1->BRR = 833u;
    USART1->CTLR1 = 0x200Cu;
}

static uint32_t got_e(void)
{
    uint32_t b;

    if ((USART1->STATR & 0x0020u) == 0u)
    {
        return 0u;
    }
    b = USART1->DATAR & 0xFFu;
    return (b == (uint32_t)'E') ? 1u : 0u;
}

/* 等到字母 E 就返回 1。这一秒里没收到就返回 0。字母只在这里读一次。 */
static uint32_t pause_or_e(void)
{
    volatile uint32_t n;

    for (n = 0u; n < 12000000u; n++)
    {
        if (got_e() != 0u)
        {
            return 1u;
        }
    }
    return 0u;
}

static void flash_unlock(void)
{
    if ((FLASH->CTLR & S5_LOCK) != 0u)
    {
        FLASH->KEYR = S5_KEY1;
        FLASH->KEYR = S5_KEY2;
    }
    if ((FLASH->CTLR & S5_FLOCK) != 0u)
    {
        FLASH->MODEKEYR = S5_KEY1;
        FLASH->MODEKEYR = S5_KEY2;
    }
}

static void flash_lock(void)
{
    FLASH->CTLR |= S5_FLOCK;
    FLASH->CTLR |= S5_LOCK;
}

static uint32_t page_is(uint32_t expect, uint32_t with_index)
{
    uint32_t i;
    uint32_t want;

    for (i = 0u; i < 64u; i++)
    {
        want = (with_index != 0u) ? (expect | i) : expect;
        if (*(volatile uint32_t *)(S5_PAGE + (i * 4u)) != want)
        {
            return 0u;
        }
    }
    return 1u;
}

/* 返回 0 成功。非 0 时 g_s5_t 已写上，控制器的忙位是清的。 */
static uint32_t erase_page(void)
{
    uint32_t st;

    FLASH->CTLR |= S5_PAGE_ER;
    FLASH->ADDR = S5_PAGE;
    FLASH->CTLR |= S5_STRT;
    st = ram_wait(&FLASH->STATR, S5_BSY, S5_SPINS);
    FLASH->CTLR &= ~S5_PAGE_ER;
    if (st != 0u)
    {
        g_s5_t = st;
    }
    return st;
}

static uint32_t program_page(void)
{
    uint32_t i;
    uint32_t st;

    FLASH->CTLR |= S5_PAGE_PG;
    st = ram_wait(&FLASH->STATR, S5_BSY, S5_SPINS);
    if (st != 0u)
    {
        FLASH->CTLR &= ~S5_PAGE_PG;
        g_s5_t = st;
        return st;
    }
    for (i = 0u; i < 64u; i++)
    {
        st = ram_wait(&FLASH->STATR, S5_WR_BSY, S5_SPINS);
        if (st != 0u)
        {
            FLASH->CTLR &= ~S5_PAGE_PG;
            g_s5_t = st;
            return st;
        }
        *(volatile uint32_t *)(S5_PAGE + (i * 4u)) = S5_TAG | i;
    }
    st = ram_wait(&FLASH->STATR, S5_WR_BSY, S5_SPINS);
    if (st != 0u)
    {
        FLASH->CTLR &= ~S5_PAGE_PG;
        g_s5_t = st;
        return st;
    }
    FLASH->CTLR |= S5_PG_STRT;
    st = ram_wait(&FLASH->STATR, S5_BSY, S5_SPINS);
    FLASH->CTLR &= ~S5_PAGE_PG;
    if (st != 0u)
    {
        g_s5_t = st;
    }
    return st;
}

static void do_once(void)
{
    tx_mark('G');
    flash_unlock();
    if (erase_page() == 0u)
    {
        g_s5_x = *(volatile uint32_t *)S5_PAGE;
        g_s5_b = page_is(S5_ERASED, 0u);
        if (program_page() == 0u)
        {
            g_s5_v = *(volatile uint32_t *)S5_PAGE;
            g_s5_k = page_is(S5_TAG, 1u);
        }
    }
    flash_lock();
    if ((g_s5_b != 0u) && (g_s5_k != 0u))
    {
        g_s5_r = 1u;
    }
    g_s5_done = 1u;
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

    uart_init();
    n = S5_N;

    while (1)
    {
        tx_mark('1');
        for (i = 0u; i < n; i++)
        {
            tx_val('A', s_addr[i]);
            tx_val('D', *(volatile uint32_t *)s_addr[i]);
        }
        if (g_s5_done != 0u)
        {
            tx_val('X', g_s5_x);
            tx_val('B', g_s5_b);
            tx_val('V', g_s5_v);
            tx_val('K', g_s5_k);
            if (g_s5_t != 0u)
            {
                tx_val('T', g_s5_t);
            }
            tx_val('R', g_s5_r);
        }
        else
        {
            tx_mark('W');
        }
        if ((g_s5_done == 0u) && (pause_or_e() != 0u))
        {
            do_once();
        }
        else if (g_s5_done != 0u)
        {
            pause_or_e();
        }
    }
}
