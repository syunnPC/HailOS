#include "fileio.h"
#include "typelib.h"
#include "status.h"
#include "fat32.h"
#include "string.h"
#include "memmgr.h"

HOSstatus OpenFile(const char* FileName, file_object_t* Result)
{
    if(Result == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if(strlen(FileName) > FILENAME_MAX)
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    HOSstatus Status;

    for(size_t s = 1*SI_MI; ; s+= 1*SI_MI)
    {
        Result->Buffer = KernelAlloc(s);
        if(Result->Buffer == NULL)
        {
            return STATUS_MEMORY_ALLOCATION_FAILED;
        }

        Result->AllocatedSize = s;
        Status = ReadFile(FileName, Result->Buffer, s, &Result->FileSize);
        if(HOS_ERROR(Status))
        {
            return Status;
        }

        if(Result->FileSize < s)
        {
            return STATUS_SUCCESS;
        }

        KernelFree(Result->Buffer, s);
    }
}

HOSstatus CloseFile(file_object_t* Object)
{
    if(Object->Buffer == NULL)
    {
        return STATUS_ALREADY_SATISFIED;
    }

    KernelFree(Object->Buffer, Object->AllocatedSize);
    Object->Buffer = NULL;
    Object->FileSize = 0;
    Object->AllocatedSize = 0;

    return STATUS_SUCCESS;
}