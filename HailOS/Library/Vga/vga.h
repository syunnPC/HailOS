/*
    標準画面出力
    作成日 2025-05-29
*/

#pragma once

#include "vgatype.h"
#include "basetype.h"

/// @brief グラフィックスシステムが利用可能かクエリ
/// @return 利用可能ならtrue、利用不可能ならfalse
bool IsGraphicAvailable(void);

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

/// @brief ピクセルをバッファ上に描画（ただしフレームバッファへは転送しない）
/// @param  Location 描画するピクセルの座標
/// @param  Color ピクセルの色
/// @return Locationが画面範囲外の場合、STATUS_OUT_OF_RANGE, 正常に描画された場合はSTATUS_SUCCESS
HOSstatus DrawPixelToBuffer(coordinate_t, rgb_t);

/// @brief バッファの内容をフレームバッファへ転送
void DrawBufferContentsToFrameBuffer(void);

/// @brief バッファの内容を指定された方向にずらす
/// @param  SizePx 動かすサイズ（ピクセル単位）
/// @param  ShiftDirection 動かす方向
void ShiftBufferContents(u32, Direction);

/// @brief ピクセルを直接フレームバッファバッファ上に描画
/// @param  Location 描画するピクセルの座標
/// @param  Color ピクセルの色（フレームバッファ内の色）
void DrawPixelToRawFrame(coordinate_t, framebuffer_color_t);

/// @brief バッファをリセット
void ClearBuffer(void);

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

/// @brief 画面の解像度を取得
/// @return 画面解像度
rectangle_t GetScreenResolution(void);

/// @brief バッファ上のピクセルを消去
/// @param  Location ピクセルの座標
void SetEmptyPixelOnBuffer(coordinate_t);

/// @brief バッファ内容をシフトしてフレームバッファへ転送
/// @param SizePx 動かすサイズ（ピクセル単位）
/// @param  ShiftDirection 動かす方向
void ShiftBufferContentsAndDraw(u32, Direction);

/// @brief PPSLを取得
/// @return PPSL
u32 GetPixelPerScanLine(void);

/// @brief フレームバッファのアドレスを取得
/// @return フレームバッファのアドレス
void* GetFrameBufferRawAddress(void);