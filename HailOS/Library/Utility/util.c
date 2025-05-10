#include "util.h"
#include "Common/commonlib.h"
#include "Common/typelib.h"
#include "kerneltype.h"

NORETURN void HaltProcessor(void)
{
    while(true)
    {
        asm volatile("hlt");
    }
}

NORETURN void Panic(u32 Code, u32 Param1, u32 Param2)
{
    Reboot();
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