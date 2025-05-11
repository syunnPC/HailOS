#include "hailkrnl.h"
#include "Utility/util.h"
#include "boot.h"
#include "init.h"
#include "Common/commonlib.h"
#include "Common/typelib.h"
#include "Common/status.h"
#include "Vga/vga.h"
#include "Vga/print.h"

meminfo_t* gMemoryInfo;
graphic_info_t* gGraphicInfo;

void main(bootinfo_t* BootInfo)
{
    InitGDT();
    InitIDT();

    if(BootInfo->GraphicInfo == NULL || BootInfo->MemoryInfo == NULL)
    {
        Reboot();
    }

    gGraphicInfo = BootInfo->GraphicInfo;
    gMemoryInfo = BootInfo->MemoryInfo;

    Fill((rgbcolor_t){.Red = 30, .Green = 144, .Blue = 188});

    HaltProcessor();
}