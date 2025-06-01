#include "vga.h"
#include "vgatype.h"
#include "common.h"
#include "status.h"
#include "util.h"
#include "memutil.h"

static graphic_info_t* sGraphicInfo;
static u8* sBuffer;
static rgb_t sBackgroundColor;
static bool sGraphicAvailability = false;

#define NO_TRANSFER_BYTE 0xFF
#define CALC_PIXEL_OFFSET(x, y) ((y*sGraphicInfo->PixelsPerScanLine + x)*PIXEL_SIZE)

static const framebuffer_color_t NO_TRANSFER_COLOR = {NO_TRANSFER_BYTE, NO_TRANSFER_BYTE, NO_TRANSFER_BYTE, NO_TRANSFER_BYTE};

bool IsGraphicAvailable(void)
{
    return sGraphicAvailability;
}

void InitGraphics(graphic_info_t* GraphicInfo, rgb_t Color)
{
    if(GraphicInfo == NULL)
    {
        ForceReboot();
    }

    if(GraphicInfo->PixelFormat != PIXELFORMAT_RGB && GraphicInfo->PixelFormat != PIXELFORMAT_BGR)
    {
        PANIC(STATUS_UNSUPPORTED, GraphicInfo->PixelFormat);
    }

    sGraphicInfo = GraphicInfo;
    sBuffer = (u8*)AllocInitializedMemory(sGraphicInfo->FrameBufferSize, NO_TRANSFER_BYTE);
    if(sBuffer == NULL)
    {
        PANIC(STATUS_MEMORY_ALLOCATION_FAILED, sGraphicInfo->FrameBufferSize);
    }

    SetBackgroundColor(Color);
    FillScreenWithBackgroundColor();
    sGraphicAvailability = true;
}

framebuffer_color_t ConvertColor(rgb_t Color)
{
    framebuffer_color_t result = {0, 0, 0, 0};

    switch(sGraphicInfo->PixelFormat) 
    {
        case PIXELFORMAT_RGB:
            result.Color1 = Color.Red;
            result.Color2 = Color.Green;
            result.Color3 = Color.Blue;
            break;
        case PIXELFORMAT_BGR:
            result.Color1 = Color.Blue;
            result.Color2 = Color.Green;
            result.Color3 = Color.Red;
            break;
        default:
            PANIC(STATUS_UNSUPPORTED, (u64)sGraphicInfo->PixelFormat);
    }

    return result;
}

rgb_t ConvertColorReverse(framebuffer_color_t Color)
{
    rgb_t result;

    switch(sGraphicInfo->PixelFormat)
    {
        case PIXELFORMAT_RGB:
            result.Red = Color.Color1;
            result.Green = Color.Color2;
            result.Blue = Color.Color3;
            break;
        case PIXELFORMAT_BGR:
            result.Blue = Color.Color1;
            result.Green = Color.Color2;
            result.Red = Color.Color3;
            break;
        default:
            PANIC(STATUS_UNSUPPORTED, (u64)sGraphicInfo->PixelFormat);
    }

    return result;
}

HOSstatus DrawPixel(coordinate_t Location, rgb_t Color)
{
    DrawPixelToBuffer(Location, Color);
    DrawPixelToRawFrame(Location, ConvertColor(Color));
    return STATUS_SUCCESS;
}

HOSstatus DrawPixelToBuffer(coordinate_t Location, rgb_t Color)
{
    if(Location.X >= sGraphicInfo->HorizontalResolution || Location.Y >= sGraphicInfo->VerticalResolution)
    {
        return STATUS_OUT_OF_RANGE;
    }

    framebuffer_color_t dest_color = ConvertColor(Color);
    addr_t offset = CALC_PIXEL_OFFSET(Location.X, Location.Y);
    framebuffer_color_t* pixel_addr = (framebuffer_color_t*)((addr_t)sBuffer + offset);
    *pixel_addr = dest_color;

    return STATUS_SUCCESS;
}

void DrawPixelToRawFrame(coordinate_t Location, framebuffer_color_t Color)
{
    addr_t offset = CALC_PIXEL_OFFSET(Location.X, Location.Y);
    framebuffer_color_t* pixel_addr = (framebuffer_color_t*)(sGraphicInfo->FrameBufferBase + offset);
    *pixel_addr = Color;
}

void DrawBufferContentsToFrameBuffer(void)
{
    for(u32 y=0; y<sGraphicInfo->VerticalResolution; y++)
    {
        for(u32 x = 0; x < sGraphicInfo->HorizontalResolution; x++)
        {
            addr_t offset = CALC_PIXEL_OFFSET(x, y);
            framebuffer_color_t* buffer_pixel_addr = (framebuffer_color_t*)((addr_t)sBuffer+offset);
            if(!MemEq(buffer_pixel_addr, &NO_TRANSFER_COLOR, PIXEL_SIZE))
            {
                DrawPixelToRawFrame(COORD(x,y), *buffer_pixel_addr);
            }
        }
    }
}

void ClearBuffer(void)
{
    FillMemory(sBuffer, sGraphicInfo->FrameBufferSize, NO_TRANSFER_BYTE);
}

void ShiftBufferContents(u32 SizePx, Direction ShiftDirection)
{
    if (!sGraphicAvailability || sGraphicInfo == NULL) return;
    if(SizePx == 0)
    {
        return;
    }

    u32 screen_height = sGraphicInfo->VerticalResolution;
    u32 screen_width = sGraphicInfo->HorizontalResolution;

    if((SizePx >= screen_height && (ShiftDirection == VERTICAL_DOWN || ShiftDirection == VERTICAL_UP)) ||
        (SizePx >= screen_width && (ShiftDirection == HORIZONTAL_LEFT || ShiftDirection == HORIZONTAL_RIGHT)))
    {
        ClearBuffer();
        return;
    }

    u8* buf = AllocInitializedMemory(sGraphicInfo->FrameBufferSize, NO_TRANSFER_BYTE);
    if(buf == NULL)
    {
        PANIC(STATUS_MEMORY_ALLOCATION_FAILED, sGraphicInfo->FrameBufferSize);
    }

    u32 begin_x, end_x, begin_y, end_y; 
    i64 delta_x, delta_y;

    switch(ShiftDirection)
    {
        case VERTICAL_UP:
            begin_x = 0;
            end_x = screen_width;
            begin_y = SizePx;
            end_y = screen_height;
            delta_x = 0;
            delta_y = -((i64)SizePx);
            break;
        case VERTICAL_DOWN:
            begin_x = 0;
            end_x = screen_width;
            begin_y = 0;
            end_y = screen_height - SizePx;
            delta_x = 0;
            delta_y = (i64)SizePx;
            break;
        case HORIZONTAL_LEFT:
            begin_x = SizePx;
            end_x = screen_width;
            begin_y = 0;
            end_y = screen_height;
            delta_x = -((i64)SizePx);
            delta_y = 0;
            break;
        case HORIZONTAL_RIGHT:
            begin_x = 0;
            end_x = screen_width - SizePx;
            begin_y = 0;
            end_y = screen_height;
            delta_x = (i64)SizePx; 
            delta_y = 0;
            break;
        default:
            FreeMemory(buf, sGraphicInfo->FrameBufferSize);
            PANIC(STATUS_INVALID_PARAMETER, (u64)ShiftDirection);
    }

    for(u32 y_src = begin_y; y_src < end_y; y_src++)
    {
        for(u32 x_src = begin_x; x_src < end_x; x_src++)
        {
            addr_t offset_src = CALC_PIXEL_OFFSET(x_src, y_src);
            addr_t offset_dest = CALC_PIXEL_OFFSET((u64)((i64)x_src + delta_x), (u64)((i64)y_src + delta_y));

            if (offset_src < sGraphicInfo->FrameBufferSize && offset_dest < sGraphicInfo->FrameBufferSize) 
            {
                framebuffer_color_t* src_pixel_addr = (framebuffer_color_t*)((addr_t)sBuffer + offset_src);
                framebuffer_color_t* dest_pixel_addr_in_temp_buf = (framebuffer_color_t*)((addr_t)buf + offset_dest);
                
                if(!MemEq(src_pixel_addr, &NO_TRANSFER_COLOR, PIXEL_SIZE))
                {
                    *dest_pixel_addr_in_temp_buf = *src_pixel_addr;
                }
            }
        }
    }

    ClearBuffer();
    MemCopy(sBuffer, buf, sGraphicInfo->FrameBufferSize);
    FreeMemory(buf, sGraphicInfo->FrameBufferSize);
}


rgb_t SetBackgroundColor(rgb_t Color)
{
    rgb_t prev_color = sBackgroundColor;
    sBackgroundColor = Color;
    return prev_color;
}

rgb_t ChangeBackgroundColor(rgb_t Color)
{
    rgb_t prev_color = sBackgroundColor;
    SetBackgroundColor(Color);
    FillScreenWithBackgroundColor();
    DrawBufferContentsToFrameBuffer();
    return prev_color;
}

void FillScreenWithBackgroundColor(void)
{
    framebuffer_color_t fb_color = ConvertColor(sBackgroundColor);

    for(u32 y=0; y<sGraphicInfo->VerticalResolution; y++)
    {
        for(u32 x = 0; x < sGraphicInfo->HorizontalResolution; x++)
        {
            DrawPixelToRawFrame(COORD(x, y), fb_color);
        }
    }
}

rectangle_t GetScreenResolution(void)
{
    return RECT(sGraphicInfo->HorizontalResolution, sGraphicInfo->VerticalResolution);
}

void SetEmptyPixelOnBuffer(coordinate_t Location)
{
    addr_t offset = CALC_PIXEL_OFFSET(Location.X, Location.Y);
    framebuffer_color_t* pixel_addr = (framebuffer_color_t*)((addr_t)sBuffer + offset);
    *pixel_addr = NO_TRANSFER_COLOR;
}


void ShiftBufferContentsAndDraw(u32 SizePx, Direction ShiftDirection)
{
    ShiftBufferContents(SizePx, ShiftDirection);
    FillScreenWithBackgroundColor();
    DrawBufferContentsToFrameBuffer();
}