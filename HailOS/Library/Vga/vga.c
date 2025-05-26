#include "vga.h"
#include "util.h"
#include "status.h"

graphic_info_t* gGraphicInfo;

boolean gGraphicAvailable;

framebuffer_color_t ConvertColor(rgbcolor_t Color)
{
    framebuffer_color_t Result = {0, 0, 0, 0};

    switch(gGraphicInfo->PixelFormat)
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
    if(Location.X > gGraphicInfo->HorizontalResolution || Location.Y > gGraphicInfo->VerticalResolution)
    {
        return STATUS_OUT_OF_RANGE;
    }

    framebuffer_color_t DstColor = ConvertColor(Color);
    addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
    framebuffer_color_t* PixelAddress = (framebuffer_color_t*)(gGraphicInfo->FrameBufferBase + Offset);
    *PixelAddress = DstColor;

    return STATUS_SUCCESS;
}