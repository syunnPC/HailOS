#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

#ifndef __GNUC__
#error GCC is required.
#endif

#ifndef PACKED_STRUCTURE
#define PACKED_STRUCTURE __attribute__((packed))
#endif

#define STATUS_SUCCESS 0
#define STATUS_ALREADY_SATISFIED 0x1

#define STATUS_ERROR 0xC0000000
#define STATUS_UNSUPPORTED 0xC0000001
#define STATUS_NOT_IMPLEMENTED 0xC0000002
#define STATUS_OUT_OF_RANGE 0xC0000003
#define STATUS_NOT_INITIALIZED 0xC0000004
#define STATUS_NOT_READY 0xC0000005
#define STATUS_NOT_FOUND 0xC0000006
#define STATUS_INVALID_PARAMETER 0xC0000007
#define STATUS_MEMORY_ALLOCATION_FAILED 0xC0000008
#define STATUS_BUFFER_TOO_SMALL 0xC0000009
#define STATUS_NOT_SUPPORTED 0xC000000A
#define STATUS_NOT_AVAILABLE 0xC000000B

#define HOS_ERROR(x) (x >= 0xC0000000)

#define KERNEL_FILENAME L"\\kernel.elf"
#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define EI_NIDENT 16
#define PT_LOAD 1

#define PIXEL_SIZE 4

typedef unsigned char u8, byte, uchar, boolean, char8;
typedef unsigned short u16, word, ushort, char16;
typedef unsigned int u32, dword, uint, char32;
typedef unsigned long long u64, qword, ulong, size_t, addr_t;

typedef signed char i8, schar;
typedef short i16;
typedef int i32;
typedef long long i64, ssize_t;

typedef u32 HOSstatus;

typedef struct
{
    u8 e_ident[EI_NIDENT];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} PACKED_STRUCTURE Elf64_Ehdr;

typedef struct 
{
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
} PACKED_STRUCTURE Elf64_Phdr;

typedef struct
{
    u8 Red;
    u8 Blue;
    u8 Green;
} PACKED_STRUCTURE rgbcolor_t;

typedef struct
{
    u8 Color1;
    u8 Color2;
    u8 Color3;
    u8 Reserved;
} PACKED_STRUCTURE framebuffer_color_t;

typedef struct
{
    i64 X;
    i64 Y;
} PACKED_STRUCTURE coordinate2D_t;

typedef enum
{
    PIXELFORMAT_BGR, PIXELFORMAT_RGB, PIXELFORMAT_INVALID
}pixelformat_t;

typedef struct
{
    addr_t FrameBufferBase;
    size_t FrameBufferSize;
    u32 PixelsPerScanLine;
    u32 HorizontalResolution;
    u32 VerticalResolution;
    pixelformat_t PixelFormat;
} PACKED_STRUCTURE graphic_info_t;

typedef struct
{
    EFI_PHYSICAL_ADDRESS Base;
    UINT64 Length;
} PACKED_STRUCTURE freeregion_t;

typedef struct
{
    freeregion_t FreeMemory[MAX_FREE_REGIONS];
    UINTN FreeRegionCount;
}PACKED_STRUCTURE meminfo_t;

typedef struct
{
    UINTN Argc;
    char** Args;
    meminfo_t* MemoryInfo;
    graphic_info_t* GraphicInfo;
} PACKED_STRUCTURE bootinfo_t;

typedef void(*kernel_entrypoint_t)(bootinfo_t*);

boolean IsInitialized;
graphic_info_t gGraphicInfo;
meminfo_t gMemoryInfo;

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

NORETURN void HaltProcessor(void)
{
    while(true)
    {
        asm volatile("hlt");
    }
}

NORETURN void Panic(u32 Code, u32 Param1, u32 Param2)
{
    Print(L"Panic %r\nParam1: %r\nParam2: %r\n\nSystem halted.");
    HaltProcessor();
}

EFI_STATUS InitializeGraphics(void)
{
    if(IsInitialized)
    {
        return STATUS_ALREADY_SATISFIED;
    }

    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;
    
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if(EFI_ERROR(Status))
    {
        return STATUS_ERROR;
    }

    gGraphicInfo.FrameBufferBase = Gop->Mode->FrameBufferBase;
    gGraphicInfo.FrameBufferSize = Gop->Mode->FrameBufferSize;
    gGraphicInfo.HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
    gGraphicInfo.VerticalResolution = Gop->Mode->Info->VerticalResolution;
    gGraphicInfo.PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
    switch(Gop->Mode->Info->PixelFormat)
    {
        case PixelRedGreenBlueReserved8BitPerColor:
            gGraphicInfo.PixelFormat = PIXELFORMAT_RGB;
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            gGraphicInfo.PixelFormat = PIXELFORMAT_BGR;
            break;
        default:
            Status = STATUS_UNSUPPORTED;
            gGraphicInfo.PixelFormat = PIXELFORMAT_INVALID;
            break;
    }

    IsInitialized = true;
    return Status;
}

framebuffer_color_t ConvertColor(rgbcolor_t Color)
{
    if(!IsInitialized)
    {
        Panic(STATUS_NOT_INITIALIZED, 1, 0);
    }

    framebuffer_color_t Result = {0, 0, 0, 0};
    switch(gGraphicInfo.PixelFormat)
    {
        case PIXELFORMAT_RGB:
            Result.Color1 = Color.Red;
            Result.Color2 = Color.Green;
            Result.Color3 = Color.Blue;
            break;
        case PIXELFORMAT_BGR:
            Result.Color1 = Color.Blue;
            Result.Color2 = Color.Green;
            Result.Color3 = Color.Red;
            break;
        default:
            Panic(STATUS_UNSUPPORTED, 1, 0);
    }

    return Result;
}

HOSstatus DrawPixel(coordinate2D_t Location, rgbcolor_t Color)
{
    if(IsInitialized == false)
    {
        return STATUS_NOT_INITIALIZED;
    }

    if(Location.X > gGraphicInfo.HorizontalResolution || Location.Y > gGraphicInfo.VerticalResolution)
    {
        return STATUS_OUT_OF_RANGE;
    }

    framebuffer_color_t DstColor = ConvertColor(Color);
    addr_t Offset = (Location.Y * gGraphicInfo.PixelsPerScanLine + Location.X) * PIXEL_SIZE;
    framebuffer_color_t* PixelAddress = (framebuffer_color_t*)(gGraphicInfo.FrameBufferBase + Offset);
    *PixelAddress = DstColor;

    return STATUS_SUCCESS;
}

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

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    UINT64 EntryPoint;
    UINTN MemoryMapSize = 0;
    UINTN MapKey, DescriptorSize;
    UINT32 DescriptorVersion;
    EFI_MEMORY_DESCRIPTOR* MemoryMap = NULL;
    bootinfo_t BootInfo = {0, NULL, &gMemoryInfo, &gGraphicInfo};

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

    MemoryMapSize += DescriptorSize*10;
    MemoryMap = AllocatePool(MemoryMapSize);
    if(MemoryMap == NULL)
    {
        Panic(1, 3, 2);
    }

    Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if(EFI_ERROR(Status))
    {
        Panic(Status, 3, 3);
    }

    Status = gBS->ExitBootServices(ImageHandle, MapKey);
    if(EFI_ERROR(Status))
    {
        Panic(Status, 4, 0);
    }

    kernel_entrypoint_t Main = (kernel_entrypoint_t)(UINTN)(EntryPoint);
    ParseFreeMemory(MemoryMap, MemoryMapSize, DescriptorSize);
    Main(&BootInfo);

    HaltProcessor();
}