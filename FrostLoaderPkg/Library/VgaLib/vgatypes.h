#pragma once

#include "Common/typelib.h"
#include "Common/commonlib.h"

#define PIXEL_SIZE 4

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