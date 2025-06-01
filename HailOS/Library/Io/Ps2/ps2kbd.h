/*
    PS/2 キーボード制御用ハンドラー・バッファ
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define KEYCODE_MASK_RELEASE 0x80
#define KEY_BUFFER_SIZE 256
#define IRQ_KEYBOARD 1

extern u8 gKeyBuffer[KEY_BUFFER_SIZE];
extern size_t gOffsetRead, gOffsetWrite;

/// @brief キーボードハンドラー
void KeyboardHandler(void);