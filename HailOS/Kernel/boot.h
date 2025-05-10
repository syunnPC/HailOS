#pragma once

#include "Common/typelib.h"
#include "Vga/vga.h"
#include "MemoryManager/memmgr.h"

#define KERNEL_FILENAME L"\\kernel.elf"
#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

typedef struct
{
    u64 Argc;
    char** Args;
    meminfo_t* MemoryInfo;
    graphic_info_t* GraphicInfo;
} PACKED_STRUCTURE bootinfo_t;