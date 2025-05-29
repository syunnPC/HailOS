/*
    標準画面出力
    作成日 2025-05-29
*/

#pragma once

#include "vgatype.h"
#include "basetype.h"

extern bool gGraphicAvailability;

/// @brief グラフィックの初期化
/// @param  GraphicInfo ブートローダから渡されたgraphic_info_tへのポインタ
/// @param  Color 画面初期化の色
void InitGraphics(graphic_info_t*, rgb_t);

/// @brief RGBカラーをフレームバッファ上の色へと変換
/// @param  Color 変換元の色
/// @return 変換された色
framebuffer_color_t ConvertColor(rgb_t);

/// @brief フレームバッファ上の色をRGBへと変換
/// @param  Color 変換元の色
/// @return 変換された色
rgb_t ConvertColorReverse(framebuffer_color_t);

/// @brief ピクセルを描画（バッファからフレームバッファへと転送を行う）
/// @param  Location 描画するピクセルの座標
/// @param  Color ピクセルの色
/// @return Locationが画面範囲外の場合、STATUS_OUT_OF_RANGE, 正常に描画された場合はSTATUS_SUCCESS
HOSstatus DrawPixel(coordinate_t, rgb_t);

/// @brief ピクセルをバッファ上に描画（ただしフレームバッファへは転送しない
/// @param  Location 描画するピクセルの座標
/// @param  Color ピクセルの色
/// @return Locationが画面範囲外の場合、STATUS_OUT_OF_RANGE, 正常に描画された場合はSTATUS_SUCCESS
HOSstatus DrawPixelToBuffer(coordinate_t, rgb_t);

/// @brief バッファの内容をフレームバッファへ転送
void DrawBufferContentsToFrameBuffer(void);

/// @brief バッファの内容を指定された方向にずらす
/// @param  Size 動かすサイズ（ピクセル単位）
/// @param  Direction 動かす方向
void ShiftBufferContents(u32, Direction);

/// @brief バッファをリセット
void CleanBuffer(void);

/// @brief バックグラウンドの色を設定（画面上の色は変わらない）
/// @param  Color 新しい色
/// @return 元の色
rgb_t SetBackgroundColor(rgb_t);

/// @brief バックグラウンドの色を設定し、フレームバッファへ転送し、バッファの内容を再び書き出す
/// @param  Color 新しい色
/// @return 元の色
rgb_t ChangeBackgroundColor(rgb_t);

/// @brief バックグラウンドの色で画面上をクリアする
void FillScreenWithBackgroundColor(void);