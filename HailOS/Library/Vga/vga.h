#pragma once

#include "typelib.h"
#include "commonlib.h"

#define PIXEL_SIZE 4

#define COORD(x, y) ((coordinate2D_t){.X = x, .Y = y})

typedef enum
{
    HORIZONTAL_RIGHT, HORIZONTAL_LEFT, VERTICAL_UP, VERTICAL_DOWN
}Direction;

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

extern rgbcolor_t gBackgroundColor;

extern boolean gGraphicAvailable;
extern graphic_info_t* gGraphicInfo;
extern graphic_info_t gBufferInfo;
extern const framebuffer_color_t NO_TRANSFER_COLOR;

void InitGraphics(graphic_info_t*, rgbcolor_t);
framebuffer_color_t ConvertColor(rgbcolor_t);
HOSstatus DrawPixel(coordinate2D_t, rgbcolor_t);
HOSstatus DrawPixelToBuffer(coordinate2D_t, rgbcolor_t);
HOSstatus DrawBufferContextToFrameBuffer();
void CleanBuffer(void);
void ShiftBufferContext(size_t, Direction);
rgbcolor_t SetBackgroundColor(rgbcolor_t);
rgbcolor_t ChangeBackgroundColor(rgbcolor_t);
void FillScreenWithBackgroundColor(void);
rgbcolor_t ConvertColorReverse(framebuffer_color_t);