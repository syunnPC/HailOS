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

    gHwClockInfo = BootInfo->ClockInfo;
    InitMemoryManager(BootInfo->MemoryInfo);
    InitGraphics(BootInfo->GraphicInfo, COLOR_AQUA);

    PUTS("HailOS version 0.0.2 Alpha\n\rSee https://github.com/syunnPC/HailOS\r\n");
    Wait(4);

    PUTS(utos(GetTotalFreeMemory()/SI_MI));
    PUTS(" MiB memory available.\r\n");
    Wait(4);

    InitVbr();
    PUTS("VBR Initialization finished.\r\n");
    Wait(4);

    HOSstatus Status;
    size_t FileSizeMax = 1*SI_MI;
    size_t FileSizeActual = 0;
    u8* buffer = KernelAlloc(FileSizeMax);
    if(buffer == NULL)
    {
        Panic(STATUS_MEMORY_ALLOCATION_FAILED, 2, __LINE__);
    }

    PUTS("Reading \"picture1.bmp\" from disk... ");
    Status = ReadFile("picture1.bmp", buffer, FileSizeMax, &FileSizeActual);
    if(HOS_ERROR(Status))
    {
        Panic(Status, 3, __LINE__);
    }
    PUTS("done! File Loaded at address ");
    PUTS(utos((addr_t)buffer));
    PUTS(", filesize [KiB] = ");
    PUTS(utos(FileSizeActual/SI_KI));

    Scroll(1);

    PUTS("Scroll is not implemented!");

    ChangeBackgroundColor(COLOR_NAVY);

    HaltProcessor();
}