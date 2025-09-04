/*
    ビットマップ画像用定義
    作成日 2025-06-01
*/

#pragma once

#include "basetype.h"
#include "vgatype.h"
#include "common.h"

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3
#define BI_JPEG 4
#define BI_PNG 5

typedef struct
{
    char bfType[2];
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
}PACKED bitmapfileheader_t;

typedef struct
{
    u32 bcSize;
    u32 bcWidth;
    i32 bcHeight;
    u16 bcPlanes;
    u16 bcBitCount;
    u32 biCompression;
    u32 biSizeImage;
    u32 biXPixPerMeter;
    u32 biYPixPerMeter;
    u32 biClrUsed;
    u32 biCirImportant;
}PACKED bitmapinfoheader_t;

typedef struct
{
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
}PACKED rgbquad_t;

/// @brief ビットマップをバッファ上の任意の位置に描画
/// @param  FileName ファイル名
/// @param  Location 描画する場所
/// @param  Rect 描画されたビットマップの形（NULLも可）
/// @return ステータスコード
HOSstatus DrawBitmapToBuffer(const char*, coordinate_t, rectangle_t*);

/// @brief ビットマップをバッファ上の任意の位置に描画し、バッファをフレームバッファへ転送
/// @param  FileName ファイル名
/// @param  Location 描画する場所
/// @param  Rect 描画されたビットマップの形（NULLも可）
/// @return ステータスコード
HOSstatus DrawBitmap(const char*, coordinate_t, rectangle_t*);

/// @brief 画像のサイズを取得
/// @param  FileName ファイル名
/// @return 画像が存在しなければ0x0、そうでなければ画像のサイズのrectangle
rectangle_t GetPictureSize(const char*);

HOSstatus BitmapToRGBArray(const char *FileName, rgb_t ***Result, rectangle_t *OutSize);