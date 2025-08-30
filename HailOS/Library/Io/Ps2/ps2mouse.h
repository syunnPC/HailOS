/*
    PS/2 マウス
    作成日 2025-08-28
*/

#pragma once

#include "basetype.h"

typedef struct
{
    int X;
    int Y;
    bool LeftButton;
    bool RightButton;
    bool MiddleButton;
} mouse_state_t;

/// @brief マウスハンドラー
void MouseHanlder(void);

/// @brief マウス初期化
bool InitMouse(void);