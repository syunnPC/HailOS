#pragma once

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include "Common/typelib.h"
#include "VgaLib/vgatypes.h"

#define KERNEL_FILENAME L"\\kernel.elf"

typedef struct
{
    UINTN Argc;
    char** Args;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    UINTN EntryCount;
    graphic_info_t* GraphicInfo;
} PACKED_STRUCTURE bootinfo_t;

typedef void(*kernel_entrypoint_t)(bootinfo_t);