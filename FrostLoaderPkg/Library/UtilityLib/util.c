#include <Uefi.h>
#include <Library/UefiLib.h>
#include "util.h"

NORETURN void HaltProcessor(void)
{
    while(true)
    {
        asm volatile("hlt");
    }
}

NORETURN void Panic(u32 Code, u32 Param1, u32 Param2)
{
    Print(L"Panic %r\nParam1: %r\nParam2: %r\n\nSystem halted.");
    HaltProcessor();
}