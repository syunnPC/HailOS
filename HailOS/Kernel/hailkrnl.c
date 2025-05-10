#include "hailkrnl.h"
#include "util.h"
#include "boot.h"
#include "init.h"
#include "Common/commonlib.h"
#include "Common/typelib.h"
#include "Common/status.h"

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


}