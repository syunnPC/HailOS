/*
    システムコンソールの操作
    作成日 2025-05-31
*/

#pragma once

#include "vgatype.h"
#include "color.h"

#define puts(str) PrintString(str, COLOR_WHITE)

/// @brief 画面上に文字を描写（バッファのフラッシュを行う）
/// @param  Ch 文字
/// @param  Color 文字色
void PrintChar(char, rgb_t);

/// @brief 画面上に文字列を描写（バッファのフラッシュを行う）
/// @param  Str 文字列
/// @param  Color 文字列色
void PrintString(const char*, rgb_t);

/// @brief 直前の文字を削除
void DeleteChar(void);

/// @brief バッファの文字列をスクロール
/// @param Line スクロールする行数
void Scroll(u32 Line);

/// @brief カーソル位置を変更
/// @param  Location 新しいカーソル位置
/// @return 変更前のカーソル位置
coordinate_t SetCursorPos(coordinate_t);

/// @brief カーソル位置を取得
/// @return 現在のカーソル位置
coordinate_t GetCursorPos(void);

/// @brief バッファ上のカーソル前文字を削除し、カーソルを移動する
void DeleteCharOnBuffer(void);

/// @brief キー入力を1つ受け付けて、表示する
/// @param Color 文字色
/// @return キーのASCIIコード
char ReadKeyWithEcho(rgb_t);

/// @brief ユーザー入力をEnterが来るかバッファサイズの上限に達するまで受け付ける（画面表示はEnterが押されるまで）
/// @param  Buffer 結果を入れるバッファ
/// @param  BufferSize バッファのサイズ、NULL文字分も含めてカウント
/// @param  Color 文字の色
/// @param NewLine Enter後に改行するか
/// @return ユーザー入力の長さ
size_t ReadInputWithEcho(char*, size_t, rgb_t, bool);

/// @brief ビットマップをインラインに描画
/// @param File ファイル名
void DrawBitmapInline(const char*);
