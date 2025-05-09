#pragma once

#include <Uefi.h>
#include "Common/commonlib.h"

#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

typedef struct
{
    EFI_PHYSICAL_ADDRESS Base;
    UINT64 Length;
} PACKED_STRUCTURE freeregion_t;

typedef struct
{
    freeregion_t FreeMemory[MAX_FREE_REGIONS];
    UINTN FreeRegionCount;
}PACKED_STRUCTURE meminfo_t;

extern meminfo_t gMemoryInfo;