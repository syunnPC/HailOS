#include "memmgr.h"

void ParseFreeMemory(EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN MemoryMapSize, UINTN DescriptorSize)
{
    UINTN EntryCount = MemoryMapSize / DescriptorSize;
    EFI_MEMORY_DESCRIPTOR* Desc = MemoryMap;
    gMemoryInfo.FreeRegionCount = 0;

    for(UINTN i=0; i<EntryCount; i++)
    {
        if(Desc->Type == EfiConventionalMemory)
        {
            gMemoryInfo.FreeMemory[gMemoryInfo.FreeRegionCount].Base = Desc->PhysicalStart;
            gMemoryInfo.FreeMemory[gMemoryInfo.FreeRegionCount].Length = Desc->NumberOfPages * PAGE_SIZE;
            gMemoryInfo.FreeRegionCount++;
        }

        Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Desc + DescriptorSize);
    }
}