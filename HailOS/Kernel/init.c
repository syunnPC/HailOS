#include "init.h"
#include "Utility/util.h"

gdt_entry_t gGDT[GDT_ENTRIES];
gdtr_t gGDTR;
idt_entry_t gIDT[IDT_ENTRIES];
idtr_t gIDTR;

void SetGDTEntry(int Idx, u32 Base, u32 Limit, u8 Access, u8 Gran)
{
    gGDT[Idx] = (gdt_entry_t){.LimitLow = Limit &0xFFFF, .BaseLow = Base & 0xFFFF, .BaseMid = (Base >> 16)&0xFF, .Access = Access, .Granularity = ((Limit >> 16)&0x0F) | (Gran & 0xF0), .BaseHigh = (Base >> 24)&0xFF};
}

void InitGDT(void)
{
    SetGDTEntry(0, 0, 0, 0, 0);                         // null descriptor
    SetGDTEntry(1, 0, 0xFFFFF, 0x9A, 0xA0);              // code segment
    SetGDTEntry(2, 0, 0xFFFFF, 0x92, 0xA0);              // data segment

    gGDTR = (gdtr_t){ .Limit = sizeof(gGDT) - 1, .Base = (u64)&gGDT };

    asm volatile ("lgdt %0" : : "m"(gGDTR));

    asm volatile (
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        // オペコードによる ljmp（FF 2E [offset:segment]）
        "jmp *%0\n"

        :
        : "m" ((const struct { u64 offset; u16 selector; }){ (u64)&&label, 0x08 })
        : "memory", "rax"
    );
label:
    return;
}

void SetIDTEntry(int Vec, handler_t Hanlder, u8 TypeAttribute)
{
    u64 Addr = (u64)Hanlder;
    gIDT[Vec] = (idt_entry_t){.OffsetLow = Addr & 0xFFFF, .Selector = 0x08, .Ist = 0, .TypeAttribute = TypeAttribute, .OffsetMid = (Addr>>16)&0xFFFF, .OffsetHigh = Addr >> 12, .Reserved = 0};
}

void InitIDT()
{
    for(int i=0; i<IDT_ENTRIES; i++)
    {
        SetIDTEntry(i, DefaultHandler, 0x8E);
    }

    SetIDTEntry(0, IsrDivideError, 0x8E);
    SetIDTEntry(6, IsrInvalidOpcode, 0x8E);
    SetIDTEntry(8, IsrDoubleFault, 0x8E);
    SetIDTEntry(13, IsrGPF, 0x8E);
    SetIDTEntry(14, IsrPageFault, 0x8E);

    gIDTR = (idtr_t){.Limit = sizeof(gIDT)-1, .Base = (u64)&gIDT};
    asm volatile ("lidt %0" : : "m"(gIDTR));
    asm volatile ("sti");
}