/*
    画面出力などに使う型やマクロの定義
    作成日 2025-05-29
*/

#pragma once

#include "common.h"
#include "basetype.h"

//ピクセルごとのフレームバッファ上のサイズ
#define PIXEL_SIZE 4
#define COORD(x, y) ((coordinate_t){x, y});

/// @brief 方向を示す（上下左右）
typedef enum
{
    HORIZONTAL_RIGHT = 1, HORIZONTAL_LEFT = 2, VERTICAL_UP = 4, VERTICAL_DOWN = 8
} Direction;

typedef struct
{
    u8 Red;
    u8 Green;
    u8 Blue
} PACKED rgb_t;

/// @brief フレームバッファ上の色情報
typedef struct
{
    u8 Color1;
    u8 Color2; 
    u8 Color3;
    u8 Color4;
} PACKED framebuffer_color_t;

/// @brief 画面上の座標を示す
typedef struct
{
    u64 X;
    u64 Y;
} coordinate_t;

/// @brief ピクセルフォーマットを示す列挙型
typedef enum
{
    PIXELFORMAT_BGR, PIXELFORMAT_RGB, PIXELFORMAT_INVALID
}pixelformat_t;

/// @brief システムのグラフィックスに関連する情報を管理する型
typedef struct
{
    addr_t FrameBufferBase;
    size_t FrameBufferSize;
    u32 PixelsPerScanLine;
    u32 HorizontalResolution;
    u32 VerticalResolution;
    pixelformat_t PixelFormat;
} PACKED graphic_info_t;