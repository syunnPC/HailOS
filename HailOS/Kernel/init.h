#pragma once

#include "kerneltype.h"
#include "commonlib.h"
#include "typelib.h"
#include "status.h"

#define IRQ_IDT(x) 32+x
#define IRQ_KEYBOARD 1

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
__attribute__((naked)) void IsrKeyboard(void);

void SetIDTEntry(int, void*, u8);
void InitIDT(void);