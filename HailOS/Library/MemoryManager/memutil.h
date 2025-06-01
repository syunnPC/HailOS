/*
    メモリユーティリティ関数
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"

/// @brief メモリ確保関数
/// @param  Size 確保するサイズ（バイト）
/// @return　確保されたメモリへのポインタ, 正常に確保できなければNULL
void* Alloc(size_t);

/// @brief メモリを指定された内容で初期化する関数
/// @param  Ptr 初期化するメモリのアドレス
/// @param  Size メモリ領域のサイズ
/// @param  Ch 初期化のパターン
void FillMemory(void*, size_t, u8);

/// @brief メモリを確保し初期化する関数
/// @param  Size 確保するサイズ（バイト）
/// @param  Ch 初期化のパターン（u8）
/// @return 確保されたメモリへのポインタ, 正常に確保できなければNULL
void* AllocInitializedMemory(size_t, u8);

/// @brief 確保されたメモリを開放
/// @param  Ptr 確保されたメモリへのポインタ
/// @param  Size 確保したメモリのサイズ
void FreeMemory(void*, size_t);

/// @brief メモリコピー関数
/// @param  Dest コピー先
/// @param  Src コピー元
/// @param  Size コピーするサイズ
void MemCopy(void*, const void*, size_t);

/// @brief 利用可能な空きメモリ領域を取得
/// @return 空きメモリのサイズ（バイト）
size_t GetAvailableMemorySize(void);

/// @brief 連続して利用可能なメモリ空間の最大サイズを取得
/// @return 最も大きい空きメモリブロックのサイズ
size_t GetLargestMemoryRegion(void);

/// @brief メモリ領域が等しいかを調べる
/// @param  Mem1 メモリ領域1
/// @param  Mem2 メモリ領域2
/// @param  Size 比べるサイズ
/// @return メモリ領域1と2の内容が等しいならtrue, そうでないならfalse
bool MemEq(const void*, const void*, size_t);