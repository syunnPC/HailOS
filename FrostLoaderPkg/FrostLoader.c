#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PerformanceLib.h>
#include <Library/TimerLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

#ifndef __GNUC__
#define GCC is required.
#endif

#ifndef PACKED_STRUCTURE
#define PACKED_STRUCTURE __attribute__((packed))
#endif

//Definitions related to status

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
#define STATUS_NOT_AVAILABLE 0xC000000A

#define HOS_ERROR(x) (x >= 0xC0000000)

//Typedefs for basic types

typedef unsigned char u8, byte, uchar, boolean, char8;
typedef unsigned short u16, word, ushort, char16;
typedef unsigned int u32, dword, uint, char32;
typedef unsigned long long u64, qword, ulong, size_t, addr_t;

typedef signed char i8, schar;
typedef short i16;
typedef int i32;
typedef long long i64, ssize_t;

typedef u32 HOSstatus;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define KERNEL_FILENAME L"\\kernel.elf"
#define MAX_FREE_REGIONS 64
#define PAGE_SIZE 4096

#define EI_NIDENT 16
#define PT_LOAD 1

#define PIXEL_SIZE 4

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
    u8 Green;
    u8 Blue;
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
    u64 InitialUnixTime;
    u64 InitialTsc;
    u64 TscFreq;
}PACKED_STRUCTURE hwclockinfo_t;

typedef struct
{
    UINTN Argc;
    char8** Args;
    meminfo_t* MemoryInfo;
    graphic_info_t* GraphicInfo;
    hwclockinfo_t* ClockInfo;
} PACKED_STRUCTURE bootinfo_t;

typedef void(*kernel_entrypoint_t)(bootinfo_t*);

boolean IsInitialized = FALSE;
graphic_info_t gGraphicInfo;
meminfo_t gMemoryInfo;
hwclockinfo_t gHwClockInfo;

#define SEC_PER_MIN   ((UINTN)    60)
#define SEC_PER_HOUR  ((UINTN)  3600)
#define SEC_PER_DAY   ((UINTN) 86400)
#define EPOCH_JULIAN_DATE  2440588

//From github: TimeBaseLib.h impl

UINTN
EFIAPI
EfiGetEpochDays (
  IN  EFI_TIME  *Time
  )
{
  UINTN  a;
  UINTN  y;
  UINTN  m;
  UINTN  JulianDate; // Absolute Julian Date representation of the supplied Time
  UINTN  EpochDays;  // Number of days elapsed since EPOCH_JULIAN_DAY

  a = (14 - Time->Month) / 12;
  y = Time->Year + 4800 - a;
  m = Time->Month + (12*a) - 3;

  JulianDate = Time->Day + ((153*m + 2)/5) + (365*y) + (y/4) - (y/100) + (y/400) - 32045;

  EpochDays = JulianDate - EPOCH_JULIAN_DATE;

  return EpochDays;
}

UINTN
EFIAPI
EfiTimeToEpoch (
  IN  EFI_TIME  *Time
  )
{
  UINTN  EpochDays;  // Number of days elapsed since EPOCH_JULIAN_DAY
  UINTN  EpochSeconds;

  EpochDays = EfiGetEpochDays (Time);

  EpochSeconds = (EpochDays * SEC_PER_DAY) + ((UINTN)Time->Hour * SEC_PER_HOUR) + (Time->Minute * SEC_PER_MIN) + Time->Second;

  return EpochSeconds;
}

//end cite

NORETURN void HaltProcessor(void)
{
    while(true)
    {
        asm volatile("hlt");
    }
}

NORETURN void Panic(UINT64 Code, UINT64 Param, UINTN Line)
{
    Print(L"Panic %lu, Parameter: %lu, Line: %lu\n", Code, Param, Line);
    Print(L"System halted.\n");
    HaltProcessor();
}

EFI_STATUS LoadFileToMemory(CHAR16* FilePath, VOID** FileBuffer, UINTN* FileSize)
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_FILE_PROTOCOL* RootDir;
    EFI_FILE_PROTOCOL* File;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    EFI_FILE_INFO* FileInfo = NULL;
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
        File->Close(File);
        RootDir->Close(RootDir);
        return Status;
    }

    FileInfo = AllocatePool(FileInfoSize);
    if(FileInfo == NULL)
    {
        File->Close(File);
        RootDir->Close(RootDir);
        return EFI_OUT_OF_RESOURCES;
    }

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if(EFI_ERROR(Status))
    {
        FreePool(FileInfo);
        File->Close(File);
        RootDir->Close(RootDir);
        return Status;
    }

    *FileSize = (UINTN)FileInfo->FileSize;
    Buffer = AllocatePool(*FileSize);
    if(Buffer == NULL)
    {
        FreePool(FileInfo);
        File->Close(File);
        RootDir->Close(RootDir);
        return EFI_OUT_OF_RESOURCES;
    }

    Status = File->Read(File, FileSize, Buffer);
    if(EFI_ERROR(Status))
    {
        FreePool(FileInfo);
        FreePool(Buffer);
        File->Close(File);
        RootDir->Close(RootDir);
        return Status;
    }

    *FileBuffer = Buffer;
    FreePool(FileInfo);
    File->Close(File);
    RootDir->Close(RootDir);
    return EFI_SUCCESS;
}

EFI_STATUS LoadElfExecutable(VOID* Buffer, UINTN FileSize, UINT64* EntryPoint)
{
    Elf64_Ehdr* Ehdr = (Elf64_Ehdr*)Buffer;
    if(Ehdr->e_ident[0]!=0x7F || Ehdr->e_ident[1] != 'E' || Ehdr->e_ident[2] != 'L' || Ehdr->e_ident[3] != 'F')
    {
        return EFI_UNSUPPORTED;
    }

    Elf64_Phdr* Phdrs = (Elf64_Phdr*)((UINT8*)Buffer + Ehdr->e_phoff);

    for(UINT16 i=0; i<Ehdr->e_phnum; i++)
    {
        Elf64_Phdr* Phdr = &Phdrs[i];
        if(Phdr->p_type != PT_LOAD || Phdr->p_memsz == 0)
        {
            continue;
        }

        EFI_PHYSICAL_ADDRESS DestPhysAddr = Phdr->p_vaddr;
        UINTN Pages = EFI_SIZE_TO_PAGES(Phdr->p_memsz);
        
        EFI_STATUS Status = gBS->AllocatePages(AllocateAddress, EfiLoaderCode, Pages, &DestPhysAddr);

        if(EFI_ERROR(Status) || DestPhysAddr != Phdr->p_vaddr)
        {
            return EFI_NOT_FOUND;
        }

        CopyMem((VOID*)(UINTN)DestPhysAddr, (UINT8*)Buffer + Phdr->p_offset, (UINTN)Phdr->p_filesz);
        if (Phdr->p_memsz > Phdr->p_filesz)
        {
            SetMem((VOID*)(UINTN)(DestPhysAddr + Phdr->p_filesz), (UINTN)(Phdr->p_memsz - Phdr->p_filesz), 0);
        }
    }
    
    *EntryPoint = Ehdr->e_entry;
    return EFI_SUCCESS;
}

EFI_STATUS PrepareElfExecutable(CHAR16* FileName, UINT64* EntryPoint)
{
    EFI_STATUS Status;
    VOID* Buffer = NULL;
    UINTN Size = 0;

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

    FreePool(Buffer);
    return EFI_SUCCESS;
}

EFI_STATUS InitializeGraphics(void)
{
    if(IsInitialized)
    {
        return EFI_SUCCESS;
    }

    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    gGraphicInfo.FrameBufferBase = Gop->Mode->FrameBufferBase;
    gGraphicInfo.FrameBufferSize = Gop->Mode->FrameBufferSize;
    gGraphicInfo.HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
    gGraphicInfo.VerticalResolution = Gop->Mode->Info->VerticalResolution;
    gGraphicInfo.PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
    Status = EFI_SUCCESS;
    
    switch(Gop->Mode->Info->PixelFormat)
    {
        case PixelRedGreenBlueReserved8BitPerColor:
            gGraphicInfo.PixelFormat = PIXELFORMAT_RGB;
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            gGraphicInfo.PixelFormat = PIXELFORMAT_BGR;
            break;
        default:
            Status = EFI_UNSUPPORTED;
            gGraphicInfo.PixelFormat = PIXELFORMAT_INVALID;
            break;
    }

    if(!EFI_ERROR(Status))
    {
        IsInitialized = true;
    }

    return Status;
}

framebuffer_color_t ConvertColor(rgbcolor_t Color)
{
    if(!IsInitialized)
    {
        Panic(STATUS_NOT_INITIALIZED, 0, __LINE__);
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
            Panic(STATUS_UNSUPPORTED, 1, __LINE__);
    }

    return Result;
}

HOSstatus DrawPixel(coordinate2D_t Location, rgbcolor_t Color)
{
    if(IsInitialized == false)
    {
        Panic(STATUS_NOT_INITIALIZED, 0, __LINE__);
    }

    if(Location.X >= gGraphicInfo.HorizontalResolution || Location.Y >= gGraphicInfo.VerticalResolution || Location.X < 0 || Location.Y < 0)
    {
        return STATUS_OUT_OF_RANGE;
    }

    framebuffer_color_t DstColor = ConvertColor(Color);
    addr_t Offset = ((addr_t)Location.Y*gGraphicInfo.PixelsPerScanLine + (addr_t)Location.X)*PIXEL_SIZE;
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
            if(gMemoryInfo.FreeRegionCount < MAX_FREE_REGIONS)
            {
                gMemoryInfo.FreeMemory[gMemoryInfo.FreeRegionCount].Base = Desc->PhysicalStart;
                gMemoryInfo.FreeMemory[gMemoryInfo.FreeRegionCount].Length = Desc->NumberOfPages * PAGE_SIZE;
                gMemoryInfo.FreeRegionCount++;
            }
            else
            {
                break;
            }
        }

        Desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Desc+DescriptorSize);
    }
}

u64 ReadTsc(void)
{
    u32 lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi<<32) | lo;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable)
{
    Print(L"FrostLoader version 0.2\n\n");

    EFI_STATUS Status;
    UINT64 EntryPoint = 0;
    EFI_MEMORY_DESCRIPTOR* MemoryMap = NULL;
    UINTN MemoryMapSize = 0;
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DescriptorVesion = 0;
    EFI_TIME CurrentTime;
    u64 Start, End;
    bootinfo_t BootInfo = {0, NULL, &gMemoryInfo, &gGraphicInfo, &gHwClockInfo};

    Print(L"Initializing graphics... ");
    Status = InitializeGraphics();
    if(EFI_ERROR(Status))
    {
        Print(L"failed.\n");
        Panic(Status, 0, __LINE__);
    }
    if(gGraphicInfo.PixelFormat == PIXELFORMAT_INVALID)
    {
        Print(L"Unsupported pixel format detected!\n");
        Panic(STATUS_UNSUPPORTED, (u64)gGraphicInfo.PixelFormat, __LINE__);
    }
    Print(L"done.\n");

    Print(L"Frame buffer base: 0x%lx, size: 0x%lx\n", gGraphicInfo.FrameBufferBase, gGraphicInfo.FrameBufferSize);
    Print(L"Resolution: %ux%u, PPSL: %u, Format: %d\n", gGraphicInfo.HorizontalResolution, gGraphicInfo.VerticalResolution, gGraphicInfo.PixelsPerScanLine, gGraphicInfo.PixelFormat);

    Print(L"Initializing hardware clock...");
    gRT->GetTime(&CurrentTime, NULL);
    gHwClockInfo.InitialUnixTime = EfiTimeToEpoch(&CurrentTime);
    gHwClockInfo.InitialTsc = ReadTsc();
    Start = ReadTsc();
    gBS->Stall(1000000);
    End = ReadTsc();
    gHwClockInfo.TscFreq = End - Start;
    Print(L"done.\n");

    Print(L"Current UNIX time: %lu, Initial TSC: %lu, TSC Freq: %lu MHz\n", gHwClockInfo.InitialUnixTime, gHwClockInfo.InitialTsc, gHwClockInfo.TscFreq/(1000*1000));

    Print(L"Loading kernel '%s'... ", KERNEL_FILENAME);
    Status = PrepareElfExecutable(KERNEL_FILENAME, &EntryPoint);
    if(EFI_ERROR(Status))
    {
        Print(L"failed.\n");
        Panic(Status, 0, __LINE__);
    }
    Print(L"done.\n");
    Print(L"Kernel loaded. Entry point address: 0x%lx\n",EntryPoint);

    Print(L"Acquiring memory map and exiting boot services... ");

    while(true)
    {
        Status = gBS->GetMemoryMap(&MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVesion);
        if(Status != EFI_BUFFER_TOO_SMALL)
        {
            Print(L"failed.\n");
            Panic(Status, 0, __LINE__);
        }

        MemoryMapSize += DescriptorSize * 5;

        if(MemoryMap != NULL)
        {
            gBS->FreePool(MemoryMap);
            MemoryMap = NULL;
        }

        MemoryMap = AllocatePool(MemoryMapSize);

        if(MemoryMap == NULL)
        {
            Print(L"failed.\n");
            Panic(Status, 1, __LINE__);
        }

        Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVesion);

        if(EFI_ERROR(Status))
        {
            continue;
        }

        ParseFreeMemory(MemoryMap, MemoryMapSize, DescriptorSize);

        Status = gBS->ExitBootServices(ImageHandle, MapKey);

        if(!EFI_ERROR(Status))
        {
            break;
        }
    }

    kernel_entrypoint_t KernelMain = (kernel_entrypoint_t)EntryPoint;
    KernelMain(&BootInfo);

    HaltProcessor();
}