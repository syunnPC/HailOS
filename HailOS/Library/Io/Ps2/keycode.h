/*
    キーコードとASCII対応表
    作成日 2025-06-01
*/

#pragma once

#include "basetype.h"

#define LEFT_SHIFT_DOWN 0x2A
#define RIGHT_SHIFT_DOWN 0x36
#define LEFT_SHIFT_UP 0xAA
#define RIGHT_SHIFT_UP 0xB6
#define ENTER_KEY 0x1C

extern const char ScancodeToAsciiBasic[0x7F];

extern const char ScancodeToAsciiShift[0x7F];