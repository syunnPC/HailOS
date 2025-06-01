/*
    共通IO関数
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"

/// @brief IOポートに出力
/// @param  Port 宛先ポート
/// @param  Value 値
void outb(u16, u8);

/// @brief IOポートから1バイト読み取り
/// @param Port 読み取るポート
/// @return  値
u8 inb(u16);

/// @brief IOポートから2バイト読み取り
/// @param  Port 読み取るポート
/// @return 値
u16 inw(u16);