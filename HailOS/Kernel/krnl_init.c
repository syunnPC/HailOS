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

#define SYSTEM_DEFAULT_COLOR RGB(70, 70, 70)

void InitSystem(bootinfo_t* Info)
{
    InitGDT();
    InitIDT();
    RemapPic(PIC_MASTER_ISR_OFFSET, 0x28);
    PicUnmaskIrq(IRQ_KEYBOARD);
    asm volatile("sti");
    InitMemoryManager(Info->MemoryInfo);
    InitTime(Info->ClockInfo);
    InitGraphics(Info->GraphicInfo, SYSTEM_DEFAULT_COLOR);
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