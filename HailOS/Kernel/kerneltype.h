#pragma once
#include "Library/Common/commonlib.h"
#include "Library/Common/typelib.h"

#define IDT_SIZE 256

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

