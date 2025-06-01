#include "interrupt.h"
#include "util.h"
#include "status.h"
#include "ps2kbd.h"


__attribute__((interrupt)) void DefaultHandler(UNUSED void* Frame)
{
    PANIC(STATUS_ERROR, 0);
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
    PANIC(STATUS_ERROR, ErrorCode);
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