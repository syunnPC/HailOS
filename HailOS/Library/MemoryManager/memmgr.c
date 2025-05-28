#include "memmgr.h"
#include "typelib.h"
#include "status.h"
#include "util.h"

meminfo_t* gMemoryInfo;

void* KernelAlloc(size_t Size)
{
    if(Size == 0)
    {
        return NULL;
    }

    if(gMemoryInfo == NULL)
    {
        PANIC(STATUS_NOT_INITIALIZED, 0);
    }

    Size = (Size + KERNEL_ALLOC_ALIGN - 1) & ~(KERNEL_ALLOC_ALIGN - 1);

    for(size_t i=0; i<gMemoryInfo->FreeRegionCount; i++)
    {   
        addr_t Base = gMemoryInfo->FreeMemory[i].Base;
        u64 Length = gMemoryInfo->FreeMemory[i].Length;

        addr_t AlignedBase = (Base + KERNEL_ALLOC_ALIGN - 1) &~(KERNEL_ALLOC_ALIGN - 1);
        u64 Padding = AlignedBase - Base;

        if(Length < Size + Padding)
        {
            continue;
        }

        void* Allocated = (void*)(u64)AlignedBase;

        gMemoryInfo->FreeMemory[i].Base = AlignedBase + Size;
        gMemoryInfo->FreeMemory[i].Length = Length - (Size + Padding);

        return Allocated;
    }

    PANIC(STATUS_NO_MEMORY_AVAILABLE, gMemoryInfo->FreeRegionCount);
}

void KernelFree(void* Addr, size_t Size)
{
    if(Addr == NULL || Size == 0)
    {
        return;
    }

    if(gMemoryInfo->FreeRegionCount < MAX_FREE_REGIONS)
    {
        gMemoryInfo->FreeMemory[gMemoryInfo->FreeRegionCount++] = (freeregion_t){.Base = (u64)Addr, .Length = Size};
    }
}

void InitMemoryManager(meminfo_t* MemInfo)
{
    if(MemInfo == NULL)
    {
        Reboot();
    }

    gMemoryInfo = MemInfo;
}

size_t GetTotalFreeMemory(void)
{
    size_t result = 0;
    for(size_t i=0; i<gMemoryInfo->FreeRegionCount; i++)
    {
        result += gMemoryInfo->FreeMemory[i].Length;
    }

    return result;
}