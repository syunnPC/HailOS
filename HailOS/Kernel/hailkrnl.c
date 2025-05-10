#include "hailkrnl.h"
#include "util.h"
#include "boot.h"

meminfo_t* gMemoryInfo;
graphic_info_t* gGraphicInfo;

void main(bootinfo_t* BootInfo)
{
    if(BootInfo->GraphicInfo == NULL || BootInfo->MemoryInfo == NULL)
    {
        Reboot();
    }

    gGraphicInfo = BootInfo->GraphicInfo;
    gMemoryInfo = BootInfo->MemoryInfo;


}