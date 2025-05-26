#pragma once

#include "typelib.h"
#include "vga.h"
#include "memmgr.h"

typedef struct
{
    u64 InitialUnixTime;
    u64 InitialTsc;
    u64 TscFreq;
} PACKED_STRUCTURE hwclockinfo_t;

typedef struct
{
    u64 Argc;
    char** Args;
    meminfo_t* MemoryInfo;
    graphic_info_t* GraphicInfo;
    hwclockinfo_t* ClockInfo;
} PACKED_STRUCTURE bootinfo_t;
