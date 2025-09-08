#include "krnl_init.h"
#include "boot.h"
#include "init.h"
#include "vga.h"
#include "memmgr.h"
#include "timer.h"
#include "fat32.h"
#include "pic.h"
#include "color.h"
#include "hal_disk.h"
#include "system_console.h"
#include "string.h"
#include "cursor.h"
#include "ps2mouse.h"
#include "io.h"

#define SYSTEM_DEFAULT_COLOR RGB(60, 60, 60)

void InitSystem(bootinfo_t* Info)
{
    InitGDT();
    RemapPic(PIC_MASTER_ISR_OFFSET, 0x28);
    InitIDT();
    PicUnmaskIrq(0);
    PicUnmaskIrq(2);
    PicUnmaskIrq(IRQ_KEYBOARD);
    InitMemoryManager(Info->MemoryInfo);
    InitTime(Info->ClockInfo);
    InitGraphics(Info->GraphicInfo, SYSTEM_DEFAULT_COLOR);
    if(!InitMouse())
    {
        puts("Failed to initialize PS/2 Mouse.\r\n");
    }
    else
    {
        PicUnmaskIrq(IRQ_MOUSE);
#ifdef DEBUG
        puts("PS/2 Mouse initialization completed.\r\n");
#endif
    }
    asm volatile("sti");
    disk_type_t type = HALInitDisk();
    if(type != DISK_TYPE_NONE)
    {
        InitVbr();
    }
    else
    {
        puts("Failed to find a disk.\r\n");
    }

    puts("Welcome to HailOS\r\n");
    puts("Screen resolution: ");
    puts(utos(Info->GraphicInfo->HorizontalResolution));
    puts(" x ");
    puts(utos(Info->GraphicInfo->VerticalResolution));
    puts("\r\nCurrent UNIX Time: ");
    puts(utos(Info->ClockInfo->InitialUnixTime));
    puts("\r\n");
}