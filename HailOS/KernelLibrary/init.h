/*
    カーネル初期化に利用する構造体など
    作成日 2025-05-30
*/

#pragma once

#include "kernel_type.h"
#include "basetype.h"
#include "status.h"

#define IRQ_IDT(x) 32+x
#define IRQ_KEYBOARD 1

extern gdt_entry_t gGDT[GDT_ENTRIES];
extern gdtr_t gGDTR;
extern idt_entry_t gIDT[IDT_ENTRIES];
extern idtr_t gIDTR;

/// @brief GDTエントリをセット
/// @param  Idx インデックス
/// @param  Base ベース
/// @param  Limit リミット
/// @param  Access アクセス
/// @param  GranularityFlags 粒度
void SetGDTEntry(int, u32, u32, u8, u8);

/// @brief GDT初期化
void InitGDT(void);

/// @brief IDTエントリをセット
/// @param  Vec ベクタ
/// @param  Handler ハンドラー関数へのポインタ
/// @param  TypeAttribute 属性
void SetIDTEntry(int, void*, u8);

/// @brief IDTを初期化
void InitIDT(void);