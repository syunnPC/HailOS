#include "util.h"
#include "Common/commonlib.h"
#include "Common/typelib.h"
#include "kerneltype.h"
#include "Vga/print.h"

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
    PrintString("System Error. System halted.", (coordinate2D_t){.X = 10, .Y = 10}, (rgbcolor_t){.Red = 255, .Blue = 255, .Green = 255});
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