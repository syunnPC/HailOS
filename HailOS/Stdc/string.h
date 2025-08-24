/*
    標準Cライブラリ <string.h> の一部関数
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"

/// @brief 文字列の長さを求める
/// @param  Str 対象文字列
/// @return 文字列の長さ
size_t strlen(const char*);

/// @brief 文字列の一致をn文字目まで確かめる
/// @param Str1 文字列1
/// @param  Str2 文字列2
/// @param  Count 比較する長さ
/// @return もし等しいなら0, そうでなければ0ではない値
int strncmp(const char*, const char*, size_t);

/// @brief Str1でStr2が最初に出現する位置を検索
/// @param  Str1 もととなる文字列
/// @param  Str2 探す対象の文字列
/// @return 現れない場合, NULL, そうでなければ, 最初に現れる文字へのポインタ
char* strstr(const char*, const char*);

/// @brief int(i64)から文字列に変換
/// @param  Num 変換対象の数字
/// @return 変換された文字列
char* itos(i64);

/// @brief unsigned int(u64)から文字列に変換
/// @param  Num 変換対象の数字
/// @return 変換された文字列
char* utos(u64);

/// @brief charから16進数文字列に変換
/// @param Value 値
/// @return 変換された文字列
char* ByteToHexString(u8 Value);

/// @brief 文字列比較
/// @param  Str1 文字列1
/// @param  Str2 文字列2
/// @return 等しければ0
int strcmp(const char*, const char*);