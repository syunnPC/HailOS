#include "file.h"
#include "basetype.h"
#include "status.h"
#include "fat32.h"
#include "memutil.h"
#include "util.h"
#include "string.h"
#include "system_console.h"
#include "timer.h"

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

    HOSstatus status;
    for(size_t s = SI_MEGA ; ; s+= SI_MEGA)
    {
        Result->Buffer = (u8*)Alloc(s);
        if(Result->Buffer == NULL)
        {
            PANIC(STATUS_MEMORY_ALLOCATION_FAILED, s);
        }

        Result->AllocatedSize = s;
        status = ReadFile(FileName, Result->Buffer, s, &(Result->FileSize));
        if(HOS_ERROR(status))
        {
            FreeMemory(Result->Buffer, s);
            return status;
        }

        if(Result->FileSize < s)
        {
            return STATUS_SUCCESS;
        }

        FreeMemory(Result->Buffer, s);
    }
}

HOSstatus CloseFile(file_object_t* Object)
{
    if(Object->Buffer == NULL)
    {
        return STATUS_ALREADY_SATISFIED;
    }

    FreeMemory(Object->Buffer, Object->AllocatedSize);
    Object->Buffer = NULL;
    Object->FileSize = 0;
    Object->AllocatedSize = 0;

    return STATUS_SUCCESS;
}

bool IsExistingFile(const char* FileName)
{
    u8 buf;
    HOSstatus status = ReadFile(FileName, &buf, 1, NULL);
    if(HOS_ERROR(status))
    {
        return false;
    }
    return true;
}