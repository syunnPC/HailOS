#pragma once

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include "Common/typelib.h"
#include "VgaLib/vgatypes.h"
#include "MemoryManagerLib/memmgr.h"

#define KERNEL_FILENAME L"\\kernel.elf"
#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

typedef struct
{
    UINTN Argc;
    char** Args;
    meminfo_t* MemoryInfo;
    graphic_info_t* GraphicInfo;
} PACKED_STRUCTURE bootinfo_t;

typedef void(*kernel_entrypoint_t)(bootinfo_t);

void ParseFreeMemory(EFI_MEMORY_DESCRIPTOR*, UINTN, UINTN);