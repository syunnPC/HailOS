#pragma once
#include "Library/Common/commonlib.h"
#include "Library/Common/typelib.h"

#define IDT_ENTRIES 256
#define GDT_ENTRIES 3

typedef struct
{
    u16 Limit;
    u64 Base;
}PACKED_STRUCTURE gdtr_t;

typedef struct
{
    u16 LimitLow;
    u16 BaseLow;
    u8 BaseMid;
    u8 Access;
    u8 Granularity;
    u8 BaseHigh;
}PACKED_STRUCTURE gdt_entry_t;

typedef struct
{
    u16 Limit;
    u64 Base;
}PACKED_STRUCTURE idtr_t;

typedef struct
{
    u16 OffsetLow;
    u16 Selector;
    u8 Ist;
    u8 TypeAttribute;
    u16 OffsetMid;
    u32 OffsetHigh;
    u32 Reserved;
}PACKED_STRUCTURE idt_entry_t;

