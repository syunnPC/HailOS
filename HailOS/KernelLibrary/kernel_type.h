/*
    カーネル用型定義
    作成日 2025-05-29
*/

#pragma once

#include "common.h"
#include "basetype.h"

#define IDT_ENTRIES 256
#define GDT_ENTRIES 3

typedef struct
{
    u16 Limit;
    u64 Base;
}PACKED gdtr_t;

typedef struct
{
    u16 LimitLow;
    u16 BaseLow;
    u8 BaseMid;
    u8 Access;
    u8 Granularity;
    u8 BaseHigh;
}PACKED gdt_entry_t;

typedef struct
{
    u16 Limit;
    u64 Base;
}PACKED idtr_t;

typedef struct
{
    u16 OffsetLow;
    u16 Selector;
    u8 Ist;
    u8 TypeAttribute;
    u16 OffsetMid;
    u32 OffsetHigh;
    u32 Reserved;
}PACKED idt_entry_t;

typedef struct
{
    u32 Reserved0;
    u64 Rsp0;
    u64 Rsp1;
    u64 Rsp2;
    u64 Reserved1;
    u64 Ist1;
    u64 Ist2;
    u64 Ist3;
    u64 Ist4;
    u64 Ist5;
    u64 Ist6;
    u64 Ist7;
    u64 Reserved2;
    u16 Reserved3;
    u16 IoMapBase;
} PACKED tss_t;