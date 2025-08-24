#include "memutil.h"
#include "basetype.h"
#include "common.h"

void FillMemory(void* Ptr, size_t Size, u8 Ch)
{
    u8* temp = (u8*)Ptr;
    for(size_t i=0; i<Size; i++)
    {
        temp[i] = Ch;
    }
}

void* AllocInitializedMemory(size_t Size, u8 Ch)
{
    u8* allocated = (u8*)Alloc(Size);
    if(allocated == NULL)
    {
        return NULL;
    }
    FillMemory(allocated, Size, Ch);
    return allocated;
}

void MemCopy(void* Dest, const void* Src, size_t Size)
{
    if(Size == 0 || Dest == NULL || Src == NULL)
    {
        return;
    }
    for(size_t i=0; i<Size; i++)
    {
        ((u8*)Dest)[i] = ((const u8*)Src)[i];
    }
}

bool MemEq(const void* Mem1, const void* Mem2, size_t Size)
{
    if(Size == 0)
    {
        return true;
    }
    for(size_t i=0; i<Size; i++)
    {
        if(((const u8*)Mem2)[i] != ((const u8*)Mem1)[i])
        {
            return false;
        }
    }
    return true;
}