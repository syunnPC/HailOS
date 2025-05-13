#pragma once

#include "commonlib.h"
#include "typelib.h"

#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

typedef struct
{
    u64 Base;
    u64 Length;
} PACKED_STRUCTURE freeregion_t;

typedef struct
{
    freeregion_t FreeMemory[MAX_FREE_REGIONS];
    u64 FreeRegionCount;
} PACKED_STRUCTURE meminfo_t;