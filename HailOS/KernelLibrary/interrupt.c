#include "interrupt.h"
#include "util.h"
#include "status.h"
#include "ps2kbd.h"
#include "basetype.h"
#include "color.h"
#include "vga.h"
#include "system_console.h"
#include "string.h"
#include "pic.h"

char *utos_static_1(u64 n)
{
    static char buf[32]; // 再入不可, ただしPanic出力用なら十分
    int i = 30;
    buf[31] = '\0';

    if (n == 0)
    {
        buf[30] = '0';
        return &buf[30];
    }

    while (n > 0 && i > 0)
    {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    }
    return &buf[i + 1];
}

char *utos_static_2(u64 n)
{
    static char buf[32]; // 再入不可, ただしPanic出力用なら十分
    int i = 30;
    buf[31] = '\0';

    if (n == 0)
    {
        buf[30] = '0';
        return &buf[30];
    }

    while (n > 0 && i > 0)
    {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    }
    return &buf[i + 1];
}

char *utos_static_3(u64 n)
{
    static char buf[32]; // 再入不可, ただしPanic出力用なら十分
    int i = 30;
    buf[31] = '\0';

    if (n == 0)
    {
        buf[30] = '0';
        return &buf[30];
    }

    while (n > 0 && i > 0)
    {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    }
    return &buf[i + 1];
}

char *utos_static_4(u64 n)
{
    static char buf[32]; // 再入不可, ただしPanic出力用なら十分
    int i = 30;
    buf[31] = '\0';

    if (n == 0)
    {
        buf[30] = '0';
        return &buf[30];
    }

    while (n > 0 && i > 0)
    {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    }
    return &buf[i + 1];
}

char *utos_static_5(u64 n)
{
    static char buf[32]; // 再入不可, ただしPanic出力用なら十分
    int i = 30;
    buf[31] = '\0';

    if (n == 0)
    {
        buf[30] = '0';
        return &buf[30];
    }

    while (n > 0 && i > 0)
    {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    }
    return &buf[i + 1];
}

static inline unsigned long read_cr0(void)
{
    u64 v;
    asm volatile("mov %%cr0, %0" : "=r"(v));
    return v;
}
static inline unsigned long read_cr3(void)
{
    u64 v;
    asm volatile("mov %%cr3, %0" : "=r"(v));
    return v;
}
static inline unsigned long read_cr2(void)
{
    u64 v;
    asm volatile("mov %%cr2, %0" : "=r"(v));
    return v;
}

void DumpBytesAt(u64 addr, int len)
{
    puts("DumpBytesAt:");
    puts(utos(addr));
    puts(" : ");
    u8 *p = (u8 *)(addr_t)addr;
    for (int i = 0; i < len; i++)
    {
        u8 b = 0;
        volatile u8 temp = p[i];
        b = (u8)temp;
        puts(ByteToHexString(b));
        puts(" ");
    }
    puts("\r\n");
}

void Diagnose_RIP_target(u64 rip)
{
    u8 *p = (u8 *)(addr_t)rip;
    // 探索：RIP直後の最大16バイトで 48 C7 05 シーケンスを探す
    int found = -1;
    for (int off = 0; off < 16; off++)
    {
        if (p[off] == 0x48 && p[off + 1] == 0xC7 && p[off + 2] == 0x05)
        {
            found = off;
            break;
        }
    }
    if (found < 0)
    {
        puts("No RIP-relative C7 pattern found\n");
        return;
    }
    // disp32 は little-endian で p[found+3..found+6]
    i32 disp = *(i32 *)(addr_t)(rip + found + 3);
    // instruction length = 3 (48 C7 05) + 4 (disp32) + 4 (imm32) = 11
    u64 next_instr = rip + found + 11;
    u64 target = next_instr + (u64)disp;
    puts("Found C7 at +");
    puts(utos(found));
    puts("\r\n");
    puts("disp32 = ");
    puts(utos((u64)(u32)disp));
    puts("\r\n");
    puts("next_instr = ");
    puts(utos(next_instr));
    puts("\r\n");
    puts("target address = ");
    puts(utos(target));
    puts("\r\n");

    // dump a few bytes at target (読み出しのみ。失敗すると例外になる可能性あり)
    DumpBytesAt(target, 16);
}

void WalkPageTablesForVirt(addr_t virt, u64 cr3)
{
    // CR3はPML4テーブルの物理アドレス
    u64 pml4_base = cr3 & ~0xFFFULL;

    u64 pml4_index = (virt >> 39) & 0x1FF;
    u64 pdpt_index = (virt >> 30) & 0x1FF;
    u64 pd_index = (virt >> 21) & 0x1FF;
    u64 pt_index = (virt >> 12) & 0x1FF;

    puts("Walking page tables for virt: ");
    puts(utos((u64)virt));
    puts("\r\n");

    // PML4
    u64 *pml4 = (u64 *)(pml4_base + (pml4_index * 8));
    u64 pml4e = *pml4;
    puts("PML4E: ");
    puts(utos(pml4e));
    puts("\r\n");
    if (!(pml4e & 1))
    {
        puts("PML4E not present\r\n");
        return;
    }

    // PDPT
    u64 pdpt_base = pml4e & ~0xFFFULL;
    u64 *pdpt = (u64 *)(pdpt_base + (pdpt_index * 8));
    u64 pdpte = *pdpt;
    puts("PDPTE: ");
    puts(utos(pdpte));
    puts("\r\n");
    if (!(pdpte & 1))
    {
        puts("PDPTE not present\r\n");
        return;
    }
    if (pdpte & (1ULL << 7))
    {
        puts("1GB page mapped at PDPT\r\n");
        return;
    }

    // PD
    u64 pd_base = pdpte & ~0xFFFULL;
    u64 *pd = (u64 *)(pd_base + (pd_index * 8));
    u64 pde = *pd;
    puts("PDE: ");
    puts(utos(pde));
    puts("\r\n");
    if (!(pde & 1))
    {
        puts("PDE not present\r\n");
        return;
    }
    if (pde & (1ULL << 7))
    {
        puts("2MB page mapped at PD\r\n");
        return;
    }

    // PT
    u64 pt_base = pde & ~0xFFFULL;
    u64 *pt = (u64 *)(pt_base + (pt_index * 8));
    u64 pte = *pt;
    puts("PTE: ");
    puts(utos(pte));
    puts("\r\n");
    if (!(pte & 1))
    {
        puts("PTE not present\r\n");
        return;
    }

    u64 phys = (pte & ~0xFFFULL) | (virt & 0xFFF);
    puts("Mapped physical address: ");
    puts(utos(phys));
    puts("\r\n");
}

NORETURN void PanicHandlerEx(interrupt_frame_t* Frame, int vec)
{
    if (!IsGraphicAvailable())
    {
        ForceReboot();
    }
    SetBackgroundColor(COLOR_BLUE);
    FillScreenWithBackgroundColor();
    SetCursorPos(COORD(0, 0));
    ClearBuffer();
    puts("System Error! System halted. Unhandled exception/interrupt detected.\r\n");
    puts("Exception/Interrupt Vector=");
    puts(utos_static_5(vec));
    puts(", RSP=");
    puts(utos_static_1(Frame->Rsp));
    puts(", CR0=");
    puts(utos_static_2(read_cr0()));
    puts(", CR3=");
    puts(utos_static_3(read_cr3()));
    puts(", CR2=");
    puts(utos_static_4(read_cr2()));
    puts("\r\n");
    DumpBytesAt((u64)Frame->Rsp,32);
    DumpBytesAt((u64)Frame->Rip, 16);
    Diagnose_RIP_target((u64)Frame->Rip);
    WalkPageTablesForVirt(Frame->Rip, read_cr3());
    WalkPageTablesForVirt(Frame->Rsp, read_cr3());
    WalkPageTablesForVirt(read_cr2(), read_cr3());
    
    HaltProcessor();
}

void Handler(interrupt_frame_t* Frame, u64 Vector)
{
    PanicHandlerEx(Frame, Vector);
}

__attribute__((interrupt)) void DefaultHandler(interrupt_frame_t *Frame)
{
    PanicHandlerEx(Frame, 0);
}

__attribute__((interrupt)) void IsrDivideError(UNUSED void* Frame)
{
    PANIC(STATUS_ERROR, 1);
}

__attribute__((interrupt)) void IsrInvalidOpcode(UNUSED void* Frame)
{
    PANIC(STATUS_ERROR, 2);
}

__attribute__((interrupt)) void IsrDoubleFault(UNUSED void* Frame, u64 ErrorCode)
{
    PANIC(STATUS_DOUBLE_FAULT, ErrorCode);
}

__attribute__((interrupt)) void IsrGPF(UNUSED void* Frame, u64 ErrorCode)
{
    PANIC(STATUS_GENERAL_PROTECTION_FAULT, ErrorCode);
}

__attribute__((interrupt)) void IsrPageFault(UNUSED void* Frame, u64 ErrorCode)
{
    u64 Addr;
    asm volatile("mov %%cr2, %0" : "=r"(Addr));
    PANIC(STATUS_ERROR, ErrorCode);
}

__attribute__((naked)) void IsrKeyboard(void)
{
    asm volatile(
        "push %rax\n\t"
        "push %rcx\n\t"
        "push %rdx\n\t"
        "push %rsi\n\t"
        "push %rdi\n\t"
        "push %rbx\n\t"
        "push %rbp\n\t"
        "push %r8\n\t"
        "push %r9\n\t"
        "push %r10\n\t"
        "push %r11\n\t"
        "push %r12\n\t"
        "push %r13\n\t"
        "push %r14\n\t"
        "push %r15\n\t"

        "call KeyboardHandler\n\t"

        "pop %r15\n\t"
        "pop %r14\n\t"
        "pop %r13\n\t"
        "pop %r12\n\t"
        "pop %r11\n\t"
        "pop %r10\n\t"
        "pop %r9\n\t"
        "pop %r8\n\t"
        "pop %rbp\n\t"
        "pop %rbx\n\t"
        "pop %rdi\n\t"
        "pop %rsi\n\t"
        "pop %rdx\n\t"
        "pop %rcx\n\t"
        "pop %rax\n\t"

        "iretq\n\t"
    );
}

__attribute__((naked)) void IsrMouse(void)
{
    asm volatile(
        "push %rax\n\t"
        "push %rcx\n\t"
        "push %rdx\n\t"
        "push %rsi\n\t"
        "push %rdi\n\t"
        "push %rbx\n\t"
        "push %rbp\n\t"
        "push %r8\n\t"
        "push %r9\n\t"
        "push %r10\n\t"
        "push %r11\n\t"
        "push %r12\n\t"
        "push %r13\n\t"
        "push %r14\n\t"
        "push %r15\n\t"

        "call MouseHandler\n\t"

        "pop %r15\n\t"
        "pop %r14\n\t"
        "pop %r13\n\t"
        "pop %r12\n\t"
        "pop %r11\n\t"
        "pop %r10\n\t"
        "pop %r9\n\t"
        "pop %r8\n\t"
        "pop %rbp\n\t"
        "pop %rbx\n\t"
        "pop %rdi\n\t"
        "pop %rsi\n\t"
        "pop %rdx\n\t"
        "pop %rcx\n\t"
        "pop %rax\n\t"

        "iretq\n\t"
    );
}

__attribute__((naked)) void IsrTimer(void)
{
    asm volatile(
        "iretq"
    );
}

__attribute__((interrupt)) void IsrIRQ7(void* Frame)
{
    PicSendEoi(7);
    return;
}

#define DEFINE_DEFAULT_HANDLER(v) __attribute__((interrupt)) void Handler##v(interrupt_frame_t* Frame){ Handler(Frame, v); }
#define DEFINE_DEFAULT_HANDLER_ERRCODE(v)  __attribute__((interrupt)) void Handler##v(interrupt_frame_t *Frame, u64 ErrorCode) { Handler(Frame, v); }

DEFINE_DEFAULT_HANDLER(0)
DEFINE_DEFAULT_HANDLER(1)
DEFINE_DEFAULT_HANDLER(2)
DEFINE_DEFAULT_HANDLER(3)
DEFINE_DEFAULT_HANDLER(4)
DEFINE_DEFAULT_HANDLER(5)
DEFINE_DEFAULT_HANDLER(6)
DEFINE_DEFAULT_HANDLER(7)
DEFINE_DEFAULT_HANDLER_ERRCODE(8)
DEFINE_DEFAULT_HANDLER(9)
DEFINE_DEFAULT_HANDLER_ERRCODE(10)
DEFINE_DEFAULT_HANDLER_ERRCODE(11)
DEFINE_DEFAULT_HANDLER_ERRCODE(12)
DEFINE_DEFAULT_HANDLER_ERRCODE(13)
DEFINE_DEFAULT_HANDLER_ERRCODE(14)
DEFINE_DEFAULT_HANDLER(15)
DEFINE_DEFAULT_HANDLER(16)
DEFINE_DEFAULT_HANDLER_ERRCODE(17)
DEFINE_DEFAULT_HANDLER(18)
DEFINE_DEFAULT_HANDLER(19)
DEFINE_DEFAULT_HANDLER(20)
DEFINE_DEFAULT_HANDLER(21)
DEFINE_DEFAULT_HANDLER(22)
DEFINE_DEFAULT_HANDLER(23)
DEFINE_DEFAULT_HANDLER(24)
DEFINE_DEFAULT_HANDLER(25)
DEFINE_DEFAULT_HANDLER(26)
DEFINE_DEFAULT_HANDLER(27)
DEFINE_DEFAULT_HANDLER(28)
DEFINE_DEFAULT_HANDLER(29)
DEFINE_DEFAULT_HANDLER(30)
DEFINE_DEFAULT_HANDLER(31)
DEFINE_DEFAULT_HANDLER(32)
DEFINE_DEFAULT_HANDLER(33)
DEFINE_DEFAULT_HANDLER(34)
DEFINE_DEFAULT_HANDLER(35)
DEFINE_DEFAULT_HANDLER(36)
DEFINE_DEFAULT_HANDLER(37)
DEFINE_DEFAULT_HANDLER(38)
DEFINE_DEFAULT_HANDLER(39)
DEFINE_DEFAULT_HANDLER(40)
DEFINE_DEFAULT_HANDLER(41)
DEFINE_DEFAULT_HANDLER(42)
DEFINE_DEFAULT_HANDLER(43)
DEFINE_DEFAULT_HANDLER(44)
DEFINE_DEFAULT_HANDLER(45)
DEFINE_DEFAULT_HANDLER(46)
DEFINE_DEFAULT_HANDLER(47)
DEFINE_DEFAULT_HANDLER(48)
DEFINE_DEFAULT_HANDLER(49)
DEFINE_DEFAULT_HANDLER(50)
DEFINE_DEFAULT_HANDLER(51)
DEFINE_DEFAULT_HANDLER(52)
DEFINE_DEFAULT_HANDLER(53)
DEFINE_DEFAULT_HANDLER(54)
DEFINE_DEFAULT_HANDLER(55)
DEFINE_DEFAULT_HANDLER(56)
DEFINE_DEFAULT_HANDLER(57)
DEFINE_DEFAULT_HANDLER(58)
DEFINE_DEFAULT_HANDLER(59)
DEFINE_DEFAULT_HANDLER(60)
DEFINE_DEFAULT_HANDLER(61)
DEFINE_DEFAULT_HANDLER(62)
DEFINE_DEFAULT_HANDLER(63)
DEFINE_DEFAULT_HANDLER(64)
DEFINE_DEFAULT_HANDLER(65)
DEFINE_DEFAULT_HANDLER(66)
DEFINE_DEFAULT_HANDLER(67)
DEFINE_DEFAULT_HANDLER(68)
DEFINE_DEFAULT_HANDLER(69)
DEFINE_DEFAULT_HANDLER(70)
DEFINE_DEFAULT_HANDLER(71)
DEFINE_DEFAULT_HANDLER(72)
DEFINE_DEFAULT_HANDLER(73)
DEFINE_DEFAULT_HANDLER(74)
DEFINE_DEFAULT_HANDLER(75)
DEFINE_DEFAULT_HANDLER(76)
DEFINE_DEFAULT_HANDLER(77)
DEFINE_DEFAULT_HANDLER(78)
DEFINE_DEFAULT_HANDLER(79)
DEFINE_DEFAULT_HANDLER(80)
DEFINE_DEFAULT_HANDLER(81)
DEFINE_DEFAULT_HANDLER(82)
DEFINE_DEFAULT_HANDLER(83)
DEFINE_DEFAULT_HANDLER(84)
DEFINE_DEFAULT_HANDLER(85)
DEFINE_DEFAULT_HANDLER(86)
DEFINE_DEFAULT_HANDLER(87)
DEFINE_DEFAULT_HANDLER(88)
DEFINE_DEFAULT_HANDLER(89)
DEFINE_DEFAULT_HANDLER(90)
DEFINE_DEFAULT_HANDLER(91)
DEFINE_DEFAULT_HANDLER(92)
DEFINE_DEFAULT_HANDLER(93)
DEFINE_DEFAULT_HANDLER(94)
DEFINE_DEFAULT_HANDLER(95)
DEFINE_DEFAULT_HANDLER(96)
DEFINE_DEFAULT_HANDLER(97)
DEFINE_DEFAULT_HANDLER(98)
DEFINE_DEFAULT_HANDLER(99)
DEFINE_DEFAULT_HANDLER(100)
DEFINE_DEFAULT_HANDLER(101)
DEFINE_DEFAULT_HANDLER(102)
DEFINE_DEFAULT_HANDLER(103)
DEFINE_DEFAULT_HANDLER(104)
DEFINE_DEFAULT_HANDLER(105)
DEFINE_DEFAULT_HANDLER(106)
DEFINE_DEFAULT_HANDLER(107)
DEFINE_DEFAULT_HANDLER(108)
DEFINE_DEFAULT_HANDLER(109)
DEFINE_DEFAULT_HANDLER(110)
DEFINE_DEFAULT_HANDLER(111)
DEFINE_DEFAULT_HANDLER(112)
DEFINE_DEFAULT_HANDLER(113)
DEFINE_DEFAULT_HANDLER(114)
DEFINE_DEFAULT_HANDLER(115)
DEFINE_DEFAULT_HANDLER(116)
DEFINE_DEFAULT_HANDLER(117)
DEFINE_DEFAULT_HANDLER(118)
DEFINE_DEFAULT_HANDLER(119)
DEFINE_DEFAULT_HANDLER(120)
DEFINE_DEFAULT_HANDLER(121)
DEFINE_DEFAULT_HANDLER(122)
DEFINE_DEFAULT_HANDLER(123)
DEFINE_DEFAULT_HANDLER(124)
DEFINE_DEFAULT_HANDLER(125)
DEFINE_DEFAULT_HANDLER(126)
DEFINE_DEFAULT_HANDLER(127)
DEFINE_DEFAULT_HANDLER(128)
DEFINE_DEFAULT_HANDLER(129)
DEFINE_DEFAULT_HANDLER(130)
DEFINE_DEFAULT_HANDLER(131)
DEFINE_DEFAULT_HANDLER(132)
DEFINE_DEFAULT_HANDLER(133)
DEFINE_DEFAULT_HANDLER(134)
DEFINE_DEFAULT_HANDLER(135)
DEFINE_DEFAULT_HANDLER(136)
DEFINE_DEFAULT_HANDLER(137)
DEFINE_DEFAULT_HANDLER(138)
DEFINE_DEFAULT_HANDLER(139)
DEFINE_DEFAULT_HANDLER(140)
DEFINE_DEFAULT_HANDLER(141)
DEFINE_DEFAULT_HANDLER(142)
DEFINE_DEFAULT_HANDLER(143)
DEFINE_DEFAULT_HANDLER(144)
DEFINE_DEFAULT_HANDLER(145)
DEFINE_DEFAULT_HANDLER(146)
DEFINE_DEFAULT_HANDLER(147)
DEFINE_DEFAULT_HANDLER(148)
DEFINE_DEFAULT_HANDLER(149)
DEFINE_DEFAULT_HANDLER(150)
DEFINE_DEFAULT_HANDLER(151)
DEFINE_DEFAULT_HANDLER(152)
DEFINE_DEFAULT_HANDLER(153)
DEFINE_DEFAULT_HANDLER(154)
DEFINE_DEFAULT_HANDLER(155)
DEFINE_DEFAULT_HANDLER(156)
DEFINE_DEFAULT_HANDLER(157)
DEFINE_DEFAULT_HANDLER(158)
DEFINE_DEFAULT_HANDLER(159)
DEFINE_DEFAULT_HANDLER(160)
DEFINE_DEFAULT_HANDLER(161)
DEFINE_DEFAULT_HANDLER(162)
DEFINE_DEFAULT_HANDLER(163)
DEFINE_DEFAULT_HANDLER(164)
DEFINE_DEFAULT_HANDLER(165)
DEFINE_DEFAULT_HANDLER(166)
DEFINE_DEFAULT_HANDLER(167)
DEFINE_DEFAULT_HANDLER(168)
DEFINE_DEFAULT_HANDLER(169)
DEFINE_DEFAULT_HANDLER(170)
DEFINE_DEFAULT_HANDLER(171)
DEFINE_DEFAULT_HANDLER(172)
DEFINE_DEFAULT_HANDLER(173)
DEFINE_DEFAULT_HANDLER(174)
DEFINE_DEFAULT_HANDLER(175)
DEFINE_DEFAULT_HANDLER(176)
DEFINE_DEFAULT_HANDLER(177)
DEFINE_DEFAULT_HANDLER(178)
DEFINE_DEFAULT_HANDLER(179)
DEFINE_DEFAULT_HANDLER(180)
DEFINE_DEFAULT_HANDLER(181)
DEFINE_DEFAULT_HANDLER(182)
DEFINE_DEFAULT_HANDLER(183)
DEFINE_DEFAULT_HANDLER(184)
DEFINE_DEFAULT_HANDLER(185)
DEFINE_DEFAULT_HANDLER(186)
DEFINE_DEFAULT_HANDLER(187)
DEFINE_DEFAULT_HANDLER(188)
DEFINE_DEFAULT_HANDLER(189)
DEFINE_DEFAULT_HANDLER(190)
DEFINE_DEFAULT_HANDLER(191)
DEFINE_DEFAULT_HANDLER(192)
DEFINE_DEFAULT_HANDLER(193)
DEFINE_DEFAULT_HANDLER(194)
DEFINE_DEFAULT_HANDLER(195)
DEFINE_DEFAULT_HANDLER(196)
DEFINE_DEFAULT_HANDLER(197)
DEFINE_DEFAULT_HANDLER(198)
DEFINE_DEFAULT_HANDLER(199)
DEFINE_DEFAULT_HANDLER(200)
DEFINE_DEFAULT_HANDLER(201)
DEFINE_DEFAULT_HANDLER(202)
DEFINE_DEFAULT_HANDLER(203)
DEFINE_DEFAULT_HANDLER(204)
DEFINE_DEFAULT_HANDLER(205)
DEFINE_DEFAULT_HANDLER(206)
DEFINE_DEFAULT_HANDLER(207)
DEFINE_DEFAULT_HANDLER(208)
DEFINE_DEFAULT_HANDLER(209)
DEFINE_DEFAULT_HANDLER(210)
DEFINE_DEFAULT_HANDLER(211)
DEFINE_DEFAULT_HANDLER(212)
DEFINE_DEFAULT_HANDLER(213)
DEFINE_DEFAULT_HANDLER(214)
DEFINE_DEFAULT_HANDLER(215)
DEFINE_DEFAULT_HANDLER(216)
DEFINE_DEFAULT_HANDLER(217)
DEFINE_DEFAULT_HANDLER(218)
DEFINE_DEFAULT_HANDLER(219)
DEFINE_DEFAULT_HANDLER(220)
DEFINE_DEFAULT_HANDLER(221)
DEFINE_DEFAULT_HANDLER(222)
DEFINE_DEFAULT_HANDLER(223)
DEFINE_DEFAULT_HANDLER(224)
DEFINE_DEFAULT_HANDLER(225)
DEFINE_DEFAULT_HANDLER(226)
DEFINE_DEFAULT_HANDLER(227)
DEFINE_DEFAULT_HANDLER(228)
DEFINE_DEFAULT_HANDLER(229)
DEFINE_DEFAULT_HANDLER(230)
DEFINE_DEFAULT_HANDLER(231)
DEFINE_DEFAULT_HANDLER(232)
DEFINE_DEFAULT_HANDLER(233)
DEFINE_DEFAULT_HANDLER(234)
DEFINE_DEFAULT_HANDLER(235)
DEFINE_DEFAULT_HANDLER(236)
DEFINE_DEFAULT_HANDLER(237)
DEFINE_DEFAULT_HANDLER(238)
DEFINE_DEFAULT_HANDLER(239)
DEFINE_DEFAULT_HANDLER(240)
DEFINE_DEFAULT_HANDLER(241)
DEFINE_DEFAULT_HANDLER(242)
DEFINE_DEFAULT_HANDLER(243)
DEFINE_DEFAULT_HANDLER(244)
DEFINE_DEFAULT_HANDLER(245)
DEFINE_DEFAULT_HANDLER(246)
DEFINE_DEFAULT_HANDLER(247)
DEFINE_DEFAULT_HANDLER(248)
DEFINE_DEFAULT_HANDLER(249)
DEFINE_DEFAULT_HANDLER(250)
DEFINE_DEFAULT_HANDLER(251)
DEFINE_DEFAULT_HANDLER(252)
DEFINE_DEFAULT_HANDLER(253)
DEFINE_DEFAULT_HANDLER(254)
DEFINE_DEFAULT_HANDLER(255)