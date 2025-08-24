#include "util.h"
#include "basetype.h"
#include "vga.h"
#include "kernel_type.h"
#include "color.h"
#include "system_console.h"
#include "common.h"
#include "string.h"

NORETURN void Panic(HOSstatus Status, u64 Param, u32 Line)
{
    if(!IsGraphicAvailable())
    {
        ForceReboot();
    }
    SetBackgroundColor(COLOR_BLUE);
    FillScreenWithBackgroundColor();
    SetCursorPos(COORD(0,0));
    ClearBuffer();
    puts("System Error! System halted.\r\nStatus: ");
    puts(utos(Status));
    puts(", Parameter ");
    puts(utos(Param));
    puts(", at line ");
    puts(utos(Line));
    HaltProcessor();
}

NORETURN void HaltProcessor(void)
{
    while(true)
    {
        asm volatile
        (
            "cli\n"
            "hlt\n"
        );
    }
}

NORETURN void ForceReboot(void)
{
    struct {u16 Limit; u64 Base;} PACKED idtr = {0, 0};
    asm volatile
    (
        "lidt %[idtr]\n\t"
        "int3\n\t"
        "hlt\n\t"
        "jmp .\n\t"
        :
        : [idtr] "m" (idtr)
        : "memory"
    );
    HaltProcessor();
}