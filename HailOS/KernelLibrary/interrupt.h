/*
    割り込みハンドラ関数
    void* Frame - 割り込みフレーム
    (u64 ErrorCode) エラーコード
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"

__attribute__((interrupt)) void DefaultHandler(void*);
__attribute__((interrupt)) void IsrDivideError(void*);
__attribute__((interrupt)) void IsrInvalidOpcode(void*);
__attribute__((interrupt)) void IsrDoubleFault(void*, u64);
__attribute__((interrupt)) void IsrGPF(void*, u64);
__attribute__((interrupt)) void IsrPageFault(void*, u64);
__attribute__((naked)) void IsrKeyboard(void);