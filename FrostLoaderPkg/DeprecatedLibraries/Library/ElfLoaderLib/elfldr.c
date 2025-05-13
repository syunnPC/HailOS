#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>
#include "elfdef.h"
#include "elfldr.h"

EFI_STATUS LoadFileToMemory(CHAR16* FilePath, VOID** FileBuffer, UINTN* FileSize)
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_FILE_PROTOCOL* RootDir;
    EFI_FILE_PROTOCOL* File;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    EFI_FILE_INFO* FileInfo;
    UINTN FileInfoSize = 0;
    VOID* Buffer = NULL;

    Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImage);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID**)&FileSystem);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = FileSystem->OpenVolume(FileSystem, &RootDir);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = RootDir->Open(RootDir, &File, FilePath, EFI_FILE_MODE_READ, 0);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    if(Status != EFI_BUFFER_TOO_SMALL)
    {
        return Status;
    }

    FileInfo = AllocatePool(FileInfoSize);
    if(FileInfo == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if(EFI_ERROR(Status))
    {
        FreePool(FileInfo);
        return Status;
    }

    *FileSize = FileInfo->FileSize;
    Buffer = AllocatePool(*FileSize);
    if(Buffer == NULL)
    {
        FreePool(FileInfo);
        return EFI_OUT_OF_RESOURCES;
    }

    Status = File->Read(File, FileSize, Buffer);
    if(EFI_ERROR(Status))
    {
        FreePool(FileInfo);
        FreePool(Buffer);
        return Status;
    }

    *FileBuffer = Buffer;
    FreePool(FileInfo);
    return EFI_SUCCESS;
}

EFI_STATUS LoadElfExecutable(VOID* Buffer, UINTN FileSize, UINT64* EntryPoint)
{
    Elf64_Ehdr* Ehdr = (Elf64_Ehdr*)Buffer;
    if(Ehdr->e_ident[0] != 0x7F || Ehdr->e_ident[1] != 'E' || Ehdr->e_ident[2] != 'L' || Ehdr->e_ident[3] != 'F')
    {
        return EFI_LOAD_ERROR;
    }

    Elf64_Phdr* Phdrs = (Elf64_Phdr*)((UINT8*)Buffer + Ehdr->e_phoff);

    for(UINT16 i=0; i<Ehdr->e_phnum; i++)
    {
        Elf64_Phdr* Phdr = &Phdrs[i];
        if(Phdr->p_type != PT_LOAD)
        {
            continue;
        }

        EFI_PHYSICAL_ADDRESS SegmentAddr = Phdr->p_vaddr;
        UINTN Pages = EFI_SIZE_TO_PAGES(Phdr->p_memsz);

        EFI_STATUS Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, Pages, &SegmentAddr);
        if(EFI_ERROR(Status))
        {
            return Status;
        }

        CopyMem((VOID*)Phdr->p_vaddr, (UINT8*)Buffer+Phdr->p_offset, Phdr->p_filesz);
        SetMem((VOID*)(Phdr->p_vaddr + Phdr->p_filesz), Phdr->p_memsz - Phdr->p_filesz, 0);
    }

    *EntryPoint = Ehdr->e_entry;
    return EFI_SUCCESS;
}

EFI_STATUS PrepareElfExecutable(CHAR16* FileName, UINT64* EntryPoint)
{
    EFI_STATUS Status;
    VOID *Buffer;
    UINTN Size;
    
    Status = LoadFileToMemory(FileName, &Buffer, &Size);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = LoadElfExecutable(Buffer, Size, EntryPoint);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    return EFI_SUCCESS;
}