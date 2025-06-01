#include "memmgr.h"
#include "memutil.h"
#include "common.h"
#include "basetype.h"
#include "status.h"
#include "util.h"

static meminfo_t* sMemoryInfo;

#define KERNEL_ALLOC_ALIGN 8

void InitMemoryManager(meminfo_t* MemoryInfo)
{
    if(MemoryInfo == NULL)
    {
        ForceReboot();
    }

    sMemoryInfo = MemoryInfo;
}

void* Alloc(size_t Size)
{
    if(Size == 0)
    {
        return NULL;
    }

    if(sMemoryInfo == NULL)
    {
        PANIC(STATUS_NOT_INITIALIZED, 1);
    }

#ifndef KERNEL_ALLOC_NO_ALIGN //アライメントを実施しないメモリアロケーションを実施するなら定義
    Size = (Size + KERNEL_ALLOC_ALIGN - 1) & ~(KERNEL_ALLOC_ALIGN - 1); 

    for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
    {
        addr_t base_addr = sMemoryInfo->FreeMemory[i].Base;
        u64 length = sMemoryInfo->FreeMemory[i].Length;

        addr_t aligned_base = (base_addr + KERNEL_ALLOC_ALIGN - 1) &~(KERNEL_ALLOC_ALIGN - 1);
        u64 padding = aligned_base - base_addr;

        if(length < Size + padding)
        {
            continue;
        }

        void* allocated = (void*)aligned_base;

        sMemoryInfo->FreeMemory[i].Base = aligned_base+Size;
        sMemoryInfo->FreeMemory[i].Length = length - (Size+padding);
        return allocated;
    }
#else
    for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
    {
        size_t base_addr = sMemoryInfo->FreeMemory[i].Base;
        u64 length = sMemoryInfo->FreeMemory[i].Length;

        if(length < Size)
        {
            continue;
        }

        void* allocated = (void*)base_addr;
        sMemoryInfo->FreeMemory[i].Base += Size;
        sMemoryInfo->FreeMemory[i].Length -= Size;
        return allocated;
    }
#endif //KERNEL_ALLOC_NO_ALIGN

    PANIC(STATUS_NO_MEMORY_AVAILABLE, GetLargestMemoryRegion());
}

void FreeMemory(void* Ptr, size_t Size)
{
    if(Ptr == NULL || Size == 0)
    {
        return;
    }

    if(sMemoryInfo->FreeRegionCount < MAX_FREE_REGIONS)
    {
        sMemoryInfo->FreeMemory[sMemoryInfo->FreeRegionCount++] = (freeregion_t){(addr_t)Ptr, Size};
    }
}

size_t GetAvailableMemorySize(void)
{
    size_t result = 0;
    for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
    {
        result += sMemoryInfo->FreeMemory[i].Length;
    }
    return result;
}

size_t GetLargestMemoryRegion(void)
{
    size_t result = 0;
    for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
    {
        if(sMemoryInfo->FreeMemory[i].Length > result)
        {
            result = sMemoryInfo->FreeMemory[i].Length;
        }
    }
    return result;
}