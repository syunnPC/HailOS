/*
    UI用デフォルトカーソル
    作成日 2025-08-28
*/

#pragma once

#include "vgatype.h"
#include "color.h"
#include "ps2mouse.h"

#define CURSOR_DEFAULT_COLOR COLOR_BLACK

extern framebuffer_color_t* gCursorDrawBuffer;

extern const u8 Cursor[];

/// @brief カーソルを描画するのに必要な変数の初期化
void InitCursor(void);

/// @brief カーソルを指定した位置に描画
/// @param  Color 色
/// @param  Pos カーソルの位置
void UpdateCursorBuffer(rgb_t, coordinate_t);

/// @brief カーソルを隠す 
void HideCursor(void);

/// @brief カーソルの状態を取得
/// @return trueなら可視
bool GetCursorState(void);

/// @brief カーソルを表示
void ShowCursor(void);