/*
    起動時に利用する構造体など（FrostLoaderと共通）
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"
#include "common.h"
#include "vgatype.h"
#include "memdef.h"
#include "timedef.h"

typedef struct
{
    u64 Argc;
    char** Args;
    meminfo_t* MemoryInfo;
    graphic_info_t* GraphicInfo;
    hw_clockinfo_t* ClockInfo;
} PACKED bootinfo_t;