#include "krnl_init.h"
#include "boot.h"
#include "init.h"
#include "vga.h"
#include "memmgr.h"
#include "timer.h"
#include "fat32.h"
#include "pic.h"
#include "color.h"

#define SYSTEM_DEFAULT_COLOR RGB(80, 80, 80)

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
    InitVbr();
}