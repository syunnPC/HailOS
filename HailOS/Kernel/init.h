#pragma once

#include "kerneltype.h"
#include "Library/Common/commonlib.h"
#include "Library/Common/typelib.h"
#include "Library/Common/status.h"

extern gdt_entry_t gGDT[GDT_ENTRIES];
extern gdtr_t gGDTR;
extern idt_entry_t gIDT[IDT_ENTRIES];
extern idtr_t gIDTR;

#ifndef INTERRUPT
#define INTERRUPT __attribute__((interrupt))
#endif

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

typedef void(*handler_t)();

void SetGDTEntry(int, u32, u32, u8, u8);
void InitGDT(void);

INTERRUPT void DefaultHandler(void*);
INTERRUPT void IsrDivideError(void*);
INTERRUPT void IsrInvalidOpcode(void*);
INTERRUPT void IsrDoubleFault(void*, u64);
INTERRUPT void IsrGPF(void*, u64);
INTERRUPT void IsrPageFault(void*, u64);

void SetIDTEntry(int, handler_t, u8);
void InitIDT(void);