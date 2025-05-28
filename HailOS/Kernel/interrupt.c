#include "init.h"
#include "commonlib.h"
#include "util.h"
#include "pic.h"
#include "ps2kbd.h"
#include "iomgr.h"

INTERRUPT void DefaultHandler(UNUSED void* Frame)
{
    PANIC(STATUS_ERROR, 0);
}

INTERRUPT void IsrDivideError(UNUSED void* Frame)
{
    PANIC(STATUS_ERROR, 1);
}

INTERRUPT void IsrInvalidOpcode(UNUSED void* Frame)
{
    PANIC(STATUS_ERROR, 2);
}

INTERRUPT void IsrDoubleFault(UNUSED void* Frame, UNUSED u64 ErrorCode)
{
    PANIC(STATUS_ERROR, 3);
}

INTERRUPT void IsrGPF(UNUSED void* Frame, UNUSED u64 ErrorCode)
{
    PANIC(STATUS_ERROR, 4);
}

INTERRUPT void IsrPageFault(UNUSED void* Frame, UNUSED u64 ErrorCode)
{
    u64 Addr;
    asm volatile("mov %%cr2, %0" : "=r"(Addr));
    PANIC(STATUS_ERROR, (u32)ErrorCode);
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