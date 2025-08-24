/*
    カーネル初期化ルーチン
    作成日 2025-05-31
*/

#pragma once

#include "boot.h"

/// @brief システム初期化
/// @param  Info ブートローダからの構造体
void InitSystem(bootinfo_t*);