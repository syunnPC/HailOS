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
#include "pic.h"
#include "ps2kbd.h"

void main(bootinfo_t* BootInfo)
{
    InitGDT();
    InitIDT();

    gHwClockInfo = BootInfo->ClockInfo;
    InitMemoryManager(BootInfo->MemoryInfo);
    InitGraphics(BootInfo->GraphicInfo, COLOR_AQUA);

    RemapPic(PIC_MASTER_ISR_OFFSET, 0x28);
    PicUnmaskIrq(IRQ_KEYBOARD);

    asm volatile("sti");

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

    FillScreenWithBackgroundColor();
    SetCursorPos(COORD(0, 0));
    CleanBuffer();

    PUTS("File name >");
    char* buffer = ReadKeysWithEcho(FILENAME_MAX, COLOR_WHITE);
    DrawBitmap(buffer, GetCurrentCursorPos());

    while(true)
    {
        ShiftBufferContext(1, VERTICAL_UP);
        FillScreenWithBackgroundColor();
        DrawBufferContextToFrameBuffer();
        Sleep(20);
    }
    
    HaltProcessor();
}