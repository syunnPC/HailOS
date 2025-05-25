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

    Fill((rgbcolor_t){.Red = 20, .Green = 230, .Blue = 140});

    PrintString("HailOS version 0.0.1 Alpha\n\rSee https://github.com/syunnPC/HailOS", (coordinate2D_t){.X = 10, .Y = 10}, (rgbcolor_t){.Red = 255, .Green = 255, .Blue = 255});
    for(volatile size_t i=0; i<9223372036854775807; i++);

    InitVbr();

    HOSstatus Status;
    size_t FileSizeMax = 1*SI_MI;
    size_t FileSizeActual = 0;
    u8* buffer = KernelAlloc(FileSizeMax);
    if(buffer == NULL)
    {
        Fill((rgbcolor_t){.Red = 10, .Green = 200, .Blue = 150});
        Panic(STATUS_MEMORY_ALLOCATION_FAILED, 0, __LINE__);
    }

    Status = ReadFile("picture1.bmp", buffer, FileSizeMax, &FileSizeActual);
    if(HOS_ERROR(Status))
    {
        Fill((rgbcolor_t){.Red = 10, .Green = 80, .Blue = 230});
        Panic(Status, 0, __LINE__);
    }

    Fill((rgbcolor_t){.Red = 30, .Green = 144, .Blue = 188});

    HaltProcessor();
}