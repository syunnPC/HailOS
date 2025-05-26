#include "util.h"
#include "boot.h"
#include "init.h"
#include "commonlib.h"
#include "typelib.h"
#include "status.h"
#include "vga.h"
#include "print.h"
#include "fat32.h"
#include "stdcolor.h"
#include "timeinfo.h"
#include "string.h"

void main(bootinfo_t* BootInfo)
{
    InitGDT();
    InitIDT();

    if(BootInfo->GraphicInfo == NULL || BootInfo->MemoryInfo == NULL || BootInfo->ClockInfo == NULL)
    {
        Reboot();
    }

    gGraphicInfo = BootInfo->GraphicInfo;
    gMemoryInfo = BootInfo->MemoryInfo;
    gGraphicAvailable = true;
    gHwClockInfo = BootInfo->ClockInfo;

    u64 totalFreeMemory = 0;

    Fill((rgbcolor_t){.Red = 20, .Green = 230, .Blue = 140});

    PrintStringInAutoFormat("HailOS version 0.0.2 Alpha\n\rSee https://github.com/syunnPC/HailOS\r\n",COLOR_WHITE);
    Wait(4);
    for(size_t i=0; i<gMemoryInfo->FreeRegionCount; i++)
    {
        totalFreeMemory += gMemoryInfo->FreeMemory[i].Length;
    }
    PrintStringInAutoFormat(utos(totalFreeMemory), COLOR_WHITE);
    PrintStringInAutoFormat(" Bytes memory available\r\n", COLOR_WHITE);
    Wait(4);

    InitVbr();
    PrintStringInAutoFormat("VBR Initialization finished. Disk is available.\r\n", COLOR_WHITE);
    Wait(4);

    HOSstatus Status;
    size_t FileSizeMax = 1*SI_MI;
    size_t FileSizeActual = 0;
    u8* buffer = KernelAlloc(FileSizeMax); //fail to allocate - maybe ParseFreeMemory in FrostLoader is not working?
    if(buffer == NULL)
    {
        Fill((rgbcolor_t){.Red = 10, .Green = 200, .Blue = 150});
        Panic(STATUS_MEMORY_ALLOCATION_FAILED, 2, __LINE__);
    }

    Status = ReadFile("picture1.bmp", buffer, FileSizeMax, &FileSizeActual);
    if(HOS_ERROR(Status))
    {
        Fill((rgbcolor_t){.Red = 10, .Green = 80, .Blue = 230});
        Panic(Status, 3, __LINE__);
    }

    Fill((rgbcolor_t){.Red = 30, .Green = 144, .Blue = 188});

    HaltProcessor();
}