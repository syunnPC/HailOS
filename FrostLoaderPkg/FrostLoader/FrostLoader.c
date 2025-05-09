#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include "Common/status.h"
#include "VgaLib/vgalib.h"
#include "UtilityLib/util.h"
#include "ElfLoaderLib/elfldr.h"
#include "Boot/boot.h"

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    UINT64 EntryPoint;
    UINTN MemoryMapSize = 0;
    UINTN MapKey, DescriptorSize;
    UINT32 DescriptorVersion;
    EFI_MEMORY_DESCRIPTOR* MemoryMap = NULL;
    bootinfo_t BootInfo = {0, NULL, NULL,0, &gGraphicInfo};

    Status = InitializeGraphics();
    if(EFI_ERROR(Status))
    {
        Panic(Status, 1, 0);
    }

    if(gGraphicInfo.PixelFormat == PIXELFORMAT_INVALID)
    {
        Print(L"Unsupported pixel format type detected.\n");
        Panic(STATUS_UNSUPPORTED, 1, 1);
    }

    Status = PrepareElfExecutable(KERNEL_FILENAME, &EntryPoint);
    if(EFI_ERROR(Status))
    {
        Panic(Status, 2, 0);
    }

    Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if(Status != EFI_BUFFER_TOO_SMALL)
    {
        Panic(Status, 3, 1);
    }

    MemoryMap = AllocatePool(MemoryMapSize);
    if(MemoryMap == NULL)
    {
        Panic(STATUS_MEMORY_ALLOCATION_FAILED, 3, 2);
    }

    Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if(EFI_ERROR(Status))
    {
        Panic(Status, 3, 3);
    }

    BootInfo.MemoryMap = MemoryMap;

    Status = gBS->ExitBootServices(ImageHandle, MapKey);
    if(EFI_ERROR(Status))
    {
        Panic(Status, 4, 0);
    }

    kernel_entrypoint_t Main = (kernel_entrypoint_t)(UINTN)(EntryPoint);
    Main(BootInfo);

    HaltProcessor();
}
