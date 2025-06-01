/*
    メモリマネージャー型
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"
#include "common.h"

#define PAGE_SIZE 4096
#define MAX_FREE_REGIONS 64

typedef struct
{
    addr_t Base;
    size_t Length;
}PACKED freeregion_t;

typedef struct
{
    freeregion_t FreeMemory[MAX_FREE_REGIONS];
    u64 FreeRegionCount;
}PACKED meminfo_t;