/*
    PS/2 キーボード API
    作成日 2025-06-01
*/

#pragma once

#include "basetype.h"
#include "vgatype.h"

/// @brief バッファからキーを読み取り
/// @param  Out 結果を書き込むバッファー
/// @return キーが読み取れればtrue, 読み取れなければfalse
bool ReadKeyFromBuffer(u8*);

/// @brief スキャンコードをASCII文字に変換
/// @param  Scancode スキャンコード
/// @return ASCII文字に存在するキーなら変換の値、存在しないキーならばNULL文字（0）
char ScancodeToAscii(u8);

/// @brief キー入力を1つ受け付ける（エコーなし）
/// @return キーのASCIIコード
char ReadKey(void);

/// @brief ユーザー入力をEnterが来るかバッファサイズの上限に達するまで受け付ける（画面表示しない）
/// @param  Buffer 結果を入れるバッファ
/// @param  BufferSize バッファのサイズ（ヌル文字分が必要）
/// @return ユーザー入力の長さ
size_t ReadInput(char*, size_t);
