#include "util.h"
#include "basetype.h"
#include "vga.h"
#include "kernel_type.h"
#include "color.h"
#include "system_console.h"
#include "common.h"
#include "string.h"
#include "status.h"

char* utos_static(u64 n)
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

NORETURN void Panic(HOSstatus Status, u64 Param, u32 Line, const char* FileName)
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
    puts(utos_static((u64)Status));
    puts(" (");
    puts(StatusToString(Status) != NULL ? StatusToString(Status) : "unknown HOSstatus");
    puts("), Parameter ");
    puts(utos(Param));
    puts(", at line ");
    puts(utos(Line));
    puts(" in file ");
    puts(FileName);
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
    asm volatile(
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