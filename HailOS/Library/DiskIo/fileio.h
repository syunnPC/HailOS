#pragma once

#include "typelib.h"

#define FILENAME_MAX 12
#define FILESIZE_MAX (4*1000*1000*1000)

typedef struct
{
    u8* Buffer;
    size_t FileSize;
    size_t AllocatedSize;
}file_object_t;

HOSstatus OpenFile(const char*, file_object_t*);
HOSstatus CloseFile(file_object_t*);