#include "vga.h"
#include "util.h"
#include "status.h"
#include "string.h"
#include "memmgr.h"
#include "stdcolor.h"

graphic_info_t* gGraphicInfo;
graphic_info_t gBufferInfo;
rgbcolor_t gBackgroundColor;

framebuffer_color_t NO_TRANSFER_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};

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

rgbcolor_t ConvertColorReverse(framebuffer_color_t Color)
{
    rgbcolor_t Result;
    switch(gGraphicInfo->PixelFormat)
    {
        case PIXELFORMAT_RGB:
            Result.Red = Color.Color1;
            Result.Green = Color.Color2;
            Result.Blue = Color.Color3;
            break;
        case PIXELFORMAT_BGR:
            Result.Blue = Color.Color1;
            Result.Green = Color.Color2;
            Result.Red = Color.Color3;
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

HOSstatus DrawPixelToBuffer(coordinate2D_t Location, rgbcolor_t Color)
{
    if(Location.X > gGraphicInfo->HorizontalResolution || Location.Y > gGraphicInfo->VerticalResolution)
    {
        return STATUS_OUT_OF_RANGE;
    }

    framebuffer_color_t DstColor = ConvertColor(Color);
    if(memcmp(&DstColor, &NO_TRANSFER_COLOR, sizeof(framebuffer_color_t)))
    {
        return STATUS_SUCCESS;
    }
    addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
    framebuffer_color_t* PixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);
    *PixelAddress = DstColor;

    return STATUS_SUCCESS;
}

void CleanBuffer(void)
{
    for(u32 x = 0; x<gGraphicInfo->HorizontalResolution; x++)
    {  
        for(u32 y = 0; y<gGraphicInfo->VerticalResolution; y++)
        {  
            coordinate2D_t Location = {.X = x, .Y = y};
            addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
            framebuffer_color_t* PixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);
            *PixelAddress = NO_TRANSFER_COLOR;
        }
    }
}

HOSstatus DrawBufferContextToFrameBuffer()
{
    for(u32 x = 0; x<gGraphicInfo->HorizontalResolution; x++)
    {  
        for(u32 y = 0; y<gGraphicInfo->VerticalResolution; y++)
        {  
            addr_t Offset = (y* gGraphicInfo->PixelsPerScanLine + x) * PIXEL_SIZE;
            framebuffer_color_t* BufferPixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);

            if(memcmp(BufferPixelAddress, &NO_TRANSFER_COLOR, sizeof(framebuffer_color_t)) == false)
            {
                DrawPixel(COORD(x, y), ConvertColorReverse(*BufferPixelAddress));
            }
        }
    }

    return STATUS_SUCCESS;
}

void InitGraphics(graphic_info_t* GraphicInfo, rgbcolor_t Color)
{
    if(GraphicInfo == NULL)
    {
        Reboot();
    }

    gGraphicInfo = GraphicInfo;
    memcpy(&gBufferInfo, gGraphicInfo, sizeof(graphic_info_t));
    gBufferInfo.FrameBufferBase = (addr_t)KernelAlloc(GraphicInfo->FrameBufferSize);
    if(gBufferInfo.FrameBufferBase == 0)
    {
        PANIC(STATUS_MEMORY_ALLOCATION_FAILED, 1);
    }
    SetBackgroundColor(Color);
    CleanBuffer();
    FillScreenWithBackgroundColor();
    gGraphicAvailable = true;
}

void ShiftBufferContext(size_t ShiftPixel, Direction ShiftDirection)
{
    if((ShiftPixel >= gGraphicInfo->VerticalResolution && (ShiftDirection == VERTICAL_DOWN || VERTICAL_UP)) || (ShiftPixel >= gGraphicInfo->HorizontalResolution && (ShiftDirection == HORIZONTAL_LEFT || HORIZONTAL_RIGHT)))
    {
        CleanBuffer();
        return;
    }

    u8* buf = KernelAlloc(gGraphicInfo->FrameBufferSize);

    if(buf == NULL)
    {
        PANIC(STATUS_MEMORY_ALLOCATION_FAILED, 0);
    }

    for(u32 x = 0; x<gGraphicInfo->HorizontalResolution; x++)
    {  
        for(u32 y = 0; y<gGraphicInfo->VerticalResolution; y++)
        {  
            coordinate2D_t Location = {.X = x, .Y = y};
            addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
            framebuffer_color_t* PixelAddress = (framebuffer_color_t*)(buf + Offset);
            *PixelAddress = NO_TRANSFER_COLOR;
        }
    }

    switch(ShiftDirection)
    {
        case VERTICAL_UP:
            for(u32 x = 0; x<gGraphicInfo->HorizontalResolution; x++)
            {  
                for(u32 y = ShiftPixel; y<gGraphicInfo->VerticalResolution; y++)
                {  
                    if(((i64)y - (i64)ShiftPixel) < 0)
                    {
                        continue;
                    }

                    addr_t Offset = (y * gGraphicInfo->PixelsPerScanLine + x) * PIXEL_SIZE;
                    addr_t NewOffset = ((y - ShiftPixel) * gGraphicInfo->PixelsPerScanLine + x) * PIXEL_SIZE;
                    framebuffer_color_t* BufferPixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);
                    framebuffer_color_t* bufAddress = (framebuffer_color_t*)((addr_t)buf + NewOffset);
                    framebuffer_color_t temp = *BufferPixelAddress;

                    if(memcmp(BufferPixelAddress, &NO_TRANSFER_COLOR, sizeof(framebuffer_color_t)) == true)
                    {
                        continue;
                    }

                    *bufAddress = temp;
                }
            }

            CleanBuffer();
            memcpy((void*)gBufferInfo.FrameBufferBase, buf, gGraphicInfo->FrameBufferSize);
            break;
        case VERTICAL_DOWN:
            for(u32 x = 0; x<gGraphicInfo->HorizontalResolution; x++)
            {  
                for(u32 y = 0; y<gGraphicInfo->VerticalResolution - ShiftPixel; y++)
                {  
                    coordinate2D_t Location = {.X = x, .Y = y};
                    addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
                    addr_t NewOffset = ((Location.Y + ShiftPixel) * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
                    framebuffer_color_t* BufferPixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);
                    framebuffer_color_t* bufAddress = (framebuffer_color_t*)(buf + NewOffset);
                    framebuffer_color_t temp = *BufferPixelAddress;
                    if(temp.Color1 == NO_TRANSFER_COLOR.Color1 && temp.Color2 == NO_TRANSFER_COLOR.Color2 && temp.Color3 == NO_TRANSFER_COLOR.Color3 && temp.Reserved == NO_TRANSFER_COLOR.Reserved)
                    {
                        continue;
                    }

                    *bufAddress = *BufferPixelAddress;
                }
            }

            CleanBuffer();
            memcpy((u8*)gBufferInfo.FrameBufferBase, buf, gBufferInfo.FrameBufferSize);
            break;
        case HORIZONTAL_RIGHT:
            for(u32 x = ShiftPixel; x<gGraphicInfo->HorizontalResolution; x++)
            {  
                for(u32 y = 0; y<gGraphicInfo->VerticalResolution; y++)
                {  
                    coordinate2D_t Location = {.X = x, .Y = y};
                    addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
                    addr_t NewOffset = (Location.Y * gGraphicInfo->PixelsPerScanLine + (Location.X + ShiftPixel)) * PIXEL_SIZE;
                    framebuffer_color_t* BufferPixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);
                    framebuffer_color_t* bufAddress = (framebuffer_color_t*)(buf + NewOffset);

                    framebuffer_color_t temp = *BufferPixelAddress;
                    if(temp.Color1 == NO_TRANSFER_COLOR.Color1 && temp.Color2 == NO_TRANSFER_COLOR.Color2 && temp.Color3 == NO_TRANSFER_COLOR.Color3 && temp.Reserved == NO_TRANSFER_COLOR.Reserved)
                    {
                        continue;
                    }

                    *bufAddress = *BufferPixelAddress;
                }
            }

            CleanBuffer();
            memcpy((u8*)gBufferInfo.FrameBufferBase, buf, gBufferInfo.FrameBufferSize);
            break;
        case HORIZONTAL_LEFT:
            for(u32 x = 0; x<gGraphicInfo->HorizontalResolution - ShiftPixel; x++)
            {  
                for(u32 y = 0; y<gGraphicInfo->VerticalResolution; y++)
                {  
                    coordinate2D_t Location = {.X = x, .Y = y};
                    addr_t Offset = (Location.Y * gGraphicInfo->PixelsPerScanLine + Location.X) * PIXEL_SIZE;
                    addr_t NewOffset = (Location.Y * gGraphicInfo->PixelsPerScanLine + (Location.X - ShiftPixel)) * PIXEL_SIZE;
                    framebuffer_color_t* BufferPixelAddress = (framebuffer_color_t*)(gBufferInfo.FrameBufferBase + Offset);
                    framebuffer_color_t* bufAddress = (framebuffer_color_t*)(buf + NewOffset);

                    framebuffer_color_t temp = *BufferPixelAddress;
                    if(temp.Color1 == NO_TRANSFER_COLOR.Color1 && temp.Color2 == NO_TRANSFER_COLOR.Color2 && temp.Color3 == NO_TRANSFER_COLOR.Color3 && temp.Reserved == NO_TRANSFER_COLOR.Reserved)
                    {
                        continue;
                    }

                    *bufAddress = *BufferPixelAddress;
                }
            }

            CleanBuffer();
            memcpy((u8*)gBufferInfo.FrameBufferBase, buf, gBufferInfo.FrameBufferSize);
            break;
        default:
            PANIC(STATUS_INVALID_PARAMETER, (u32)ShiftDirection);
    }

    KernelFree(buf, gBufferInfo.FrameBufferSize);
}

rgbcolor_t SetBackgroundColor(rgbcolor_t Color)
{
    rgbcolor_t prevColor = gBackgroundColor;
    gBackgroundColor = Color;
    return prevColor;
}

rgbcolor_t ChangeBackgroundColor(rgbcolor_t Color)
{
    rgbcolor_t result = SetBackgroundColor(Color);
    FillScreenWithBackgroundColor();
    DrawBufferContextToFrameBuffer();
    return result;
}

void FillScreenWithBackgroundColor(void)
{
    for (i64 y = 0; y < (i64)gGraphicInfo->VerticalResolution; y++)
    {
        for (i64 x = 0; x < (i64)gGraphicInfo->HorizontalResolution; x++)
        {
            coordinate2D_t pos = { x, y };
            DrawPixel(pos, gBackgroundColor);
        }
    }
}