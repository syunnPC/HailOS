#include "util.h"
#include "boot.h"
#include "init.h"
#include "commonlib.h"
#include "typelib.h"
#include "status.h"
#include "vga.h"
#include "print.h"
#include "fat32.h"

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

    InitVbr();

    Fill((rgbcolor_t){.Red = 30, .Green = 144, .Blue = 188});

    HaltProcessor();
}