#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "UtilityLib/util.h"
#include "vgalib.h"
#include "Common/status.h"

static boolean IsInitialized;
graphic_info_t gGraphicInfo;

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