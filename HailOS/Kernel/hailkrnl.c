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
#include "stdio.h"
#include "fileio.h"
#include "bitmap.h"

void main(bootinfo_t* BootInfo)
{
    InitGDT();
    InitIDT();

    gHwClockInfo = BootInfo->ClockInfo;
    InitMemoryManager(BootInfo->MemoryInfo);
    InitGraphics(BootInfo->GraphicInfo, COLOR_AQUA);

    puts("HailOS version 0.0.2 Alpha\n\rSee https://github.com/syunnPC/HailOS");
    Wait(4);

    PUTS(utos(GetTotalFreeMemory()/SI_MI));
    PUTS(" MiB memory available. Total memory regions: ");
    puts(utos(gMemoryInfo->FreeRegionCount));
    Wait(4);

    InitVbr();
    puts("VBR Initialization finished.");
    Wait(4);

    file_object_t picture1;
    HOSstatus Status;

    PUTS("Reading \"picture1.bmp\" from disk... ");
    Status = OpenFile("picture1.bmp", &picture1);
    if(HOS_ERROR(Status))
    {
        Panic(Status, 3, __LINE__);
    }
    PUTS("done! File Loaded at address ");
    PUTS(utos((addr_t)picture1.Buffer));
    PUTS(", filesize [KiB] = ");
    puts(utos(picture1.FileSize/SI_KI));
    Status = CloseFile(&picture1);
    Wait(3);
    Scroll(2);

    FillScreenWithBackgroundColor();
    CleanBuffer();
    DrawBitmap("picture3.bmp", GetCurrentCursorPos());

    while(true)
    {
        ShiftBufferContext(10, VERTICAL_UP);
        FillScreenWithBackgroundColor();
        DrawBufferContextToFrameBuffer();
        Wait(1);
    }
    
    HaltProcessor();
}