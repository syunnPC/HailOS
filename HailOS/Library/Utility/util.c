#include "util.h"
#include "commonlib.h"
#include "typelib.h"
#include "kerneltype.h"
#include "print.h"
#include "stdcolor.h"
#include "timeinfo.h"
#include "string.h"

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
    Fill((rgbcolor_t){.Red = 41, .Green = 152, .Blue = 240});
    if(!gGraphicAvailable)
    {
        HaltProcessor();
    }
    PrintString("System Error\r\nStatus: ", (coordinate2D_t){.X = 0, .Y = 0}, COLOR_WHITE);
    PrintStringInAutoFormat(utos(Code), COLOR_WHITE);
    PrintStringInAutoFormat(" Param1: ", COLOR_WHITE);
    PrintStringInAutoFormat(utos(Param1), COLOR_WHITE);
    PrintStringInAutoFormat(" Param2: ", COLOR_WHITE);
    PrintStringInAutoFormat(utos(Param2), COLOR_WHITE);
    PrintStringInAutoFormat("\r\nSystem halted.", COLOR_WHITE);
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

void Wait(int Seconds)
{
    u64 start = ReadTsc();
    u64 endTsc = start + (Seconds * gHwClockInfo->TscFreq);
    while(ReadTsc() < endTsc);
}