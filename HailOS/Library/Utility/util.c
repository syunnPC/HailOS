#include "util.h"
#include "commonlib.h"
#include "typelib.h"
#include "kerneltype.h"
#include "print.h"
#include "stdcolor.h"
#include "timeinfo.h"
#include "string.h"
#include "vga.h"

HOSstatus gLastStatus;

NORETURN void HaltProcessor(void)
{
    while(true)
    {
        asm volatile("hlt");
    }
}

NORETURN void Panic(u32 Code, u32 Param1, u32 Param2)
{
    if(!gGraphicAvailable)
    {
        HaltProcessor();
    }
    
    CleanBuffer();
    SetBackgroundColor(COLOR_BLUE);
    FillScreenWithBackgroundColor();
    SetCursorPos(COORD(0, 0));
    PUTS("System Error\r\nStatus: ");
    PUTS(utos(Code));
    PUTS(" Param1: ");
    PUTS(utos(Param1));
    PUTS(" Param2: ");
    PUTS(utos(Param2));
    PUTS("\r\nSystem halted.");
    HaltProcessor();
}

NORETURN void Reboot(void)
{
    idtr_t idtr = {0, 0};

    asm volatile (
        "lidt %[idtr]\n\t"
        "int3\n\t"
        "hlt\n\t"
        "jmp .\n\t"
        :
        : [idtr] "m" (idtr)
        : "memory"
    );
}

void Wait(u64 Seconds)
{
    u64 Start = ReadTsc();
    u64 EndTsc = Start + (Seconds * gHwClockInfo->TscFreq);
    while(ReadTsc() < EndTsc);
}

void Sleep(u64 Miliseconds)
{
    u64 Start = ReadTsc();
    u64 EndTsc = Start + (gHwClockInfo->TscFreq * (Miliseconds / 1000));
    while(ReadTsc() < EndTsc);
}