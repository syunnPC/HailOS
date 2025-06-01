#include "init.h"
#include "util.h"
#include "interrupt.h"

gdt_entry_t gGDT[GDT_ENTRIES];
gdtr_t gGDTR;
idt_entry_t gIDT[IDT_ENTRIES];
idtr_t gIDTR;

// Access byte flags
#define GDT_ACCESS_PRESENT   0x80 // Present bit
#define GDT_ACCESS_RING0     0x00 // Ring 0
#define GDT_ACCESS_RING3     0x60 // Ring 3
#define GDT_ACCESS_CS_OR_DS  0x10 // Code or Data segment (not TSS or LDT)
#define GDT_ACCESS_EXECUTABLE 0x08 // Executable bit (for code segments)
#define GDT_ACCESS_DIRECTION 0x04 // Direction/Conforming bit (leave 0 for code)
#define GDT_ACCESS_RW        0x02 // Readable (for code), Writable (for data)

// Granularity byte flags
#define GDT_GRAN_LIMIT_HI    0x0F // Upper 4 bits of limit
#define GDT_GRAN_AVL         0x10 // Available for system use
#define GDT_GRAN_LONG_MODE   0x20 // L bit - 64-bit code segment
#define GDT_GRAN_DB          0x40 // D/B bit (0 for 64-bit code, 1 for 32-bit segments)
#define GDT_GRAN_PAGE_GRAN   0x80 // G bit - Granularity (limit in 4KB pages)

void SetGDTEntry(int Idx, u32 Base, u32 Limit, u8 Access, u8 GranularityFlags) {
    gGDT[Idx].LimitLow    = Limit & 0xFFFF;
    gGDT[Idx].BaseLow     = Base & 0xFFFF;
    gGDT[Idx].BaseMid     = (Base >> 16) & 0xFF;
    gGDT[Idx].Access      = Access;
    // Combine upper 4 bits of limit with granularity flags
    gGDT[Idx].Granularity = ((Limit >> 16) & GDT_GRAN_LIMIT_HI) | (GranularityFlags & 0xF0);
    gGDT[Idx].BaseHigh    = (Base >> 24) & 0xFF;
}

void InitGDT(void) {
    // GDT[0]: Null Descriptor
    SetGDTEntry(0, 0, 0, 0, 0);

    // GDT[1]: Kernel Code Segment (64-bit)
    // Access: Present, Ring 0, Code/Data, Executable, Read/Write (readable code)
    // Granularity: Page Granularity (G=1), Long Mode (L=1), D/B=0 (for 64-bit code)
    // Base and Limit are effectively 0 and 0xFFFFFFFFFFFFFFFF for 64-bit CS/DS.
    // We still set a "max" limit for consistency (0xFFFFF in 4KB pages -> 4GB).
    SetGDTEntry(1, 0, 0xFFFFF,
                GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CS_OR_DS | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW,
                GDT_GRAN_PAGE_GRAN | GDT_GRAN_LONG_MODE);

    // GDT[2]: Kernel Data Segment (64-bit)
    // Access: Present, Ring 0, Code/Data, Read/Write
    // Granularity: Page Granularity (G=1), D/B=1 (32-bit stack/data size, but base/limit still flat for 64-bit)
    // L-bit must be 0 for data segments.
    SetGDTEntry(2, 0, 0xFFFFF,
                GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CS_OR_DS | GDT_ACCESS_RW,
                GDT_GRAN_PAGE_GRAN | GDT_GRAN_DB); // L-bit is 0

    gGDTR.Limit = sizeof(gGDT) - 1;
    gGDTR.Base  = (u64)&gGDT;

    asm volatile ("lgdt %0" : : "m"(gGDTR));

    // Reload segment registers.
    // Use lretq for CS reload, then mov for data segments.
    asm volatile (
        "pushq $0x08 \n\t"             // Kernel Code Selector (GDT index 1)
        "leaq .L_gdt_reload_cs(%%rip), %%rax \n\t"
        "pushq %%rax \n\t"
        "lretq \n\t"
        ".L_gdt_reload_cs:"
        : : : "rax", "memory"
    );

    asm volatile (
        "mov $0x10, %%ax \n\t"         // Kernel Data Selector (GDT index 2)
        "mov %%ax, %%ds \n\t"
        "mov %%ax, %%es \n\t"
        "mov %%ax, %%ss \n\t"
        // FS and GS are typically 0 unless used for specific purposes (like TLS or syscalls)
        // Setting them to data selector is safe for now.
        "mov %%ax, %%fs \n\t"
        "mov %%ax, %%gs \n\t"
        : : : "ax"
    );
}

#define IDT_TYPE_INTERRUPT_GATE 0x8E // Present, DPL0, Interrupt Gate (32-bit or 64-bit depending on CS)
#define IDT_TYPE_TRAP_GATE      0x8F // Present, DPL0, Trap Gate

void SetIDTEntry(int Vec, void* Handler, u8 TypeAttribute) {
    u64 Addr = (u64)Handler;
    gIDT[Vec].OffsetLow     = (u16)(Addr & 0xFFFF);
    gIDT[Vec].Selector      = 0x08; // Kernel Code Segment selector
    gIDT[Vec].Ist           = 0;    // Interrupt Stack Table index (0 = use current stack)
    gIDT[Vec].TypeAttribute = TypeAttribute;
    gIDT[Vec].OffsetMid     = (u16)((Addr >> 16) & 0xFFFF);
    gIDT[Vec].OffsetHigh    = (u32)(Addr >> 32);
    gIDT[Vec].Reserved      = 0;
}

void InitIDT() {
    // Initialize all entries to a default handler first
    for(int i = 0; i < IDT_ENTRIES; i++) {
        SetIDTEntry(i, DefaultHandler, IDT_TYPE_INTERRUPT_GATE);
    }

    // Set specific handlers for common exceptions
    SetIDTEntry(0,  IsrDivideError,   IDT_TYPE_INTERRUPT_GATE); // #DE
    SetIDTEntry(6,  IsrInvalidOpcode, IDT_TYPE_INTERRUPT_GATE); // #UD
    SetIDTEntry(8,  IsrDoubleFault,   IDT_TYPE_INTERRUPT_GATE); // #DF (must be interrupt gate, error code pushed)
    SetIDTEntry(13, IsrGPF,           IDT_TYPE_INTERRUPT_GATE); // #GP (error code pushed)
    SetIDTEntry(14, IsrPageFault,     IDT_TYPE_INTERRUPT_GATE); // #PF (error code pushed)
    SetIDTEntry(IRQ_IDT(IRQ_KEYBOARD), IsrKeyboard, IDT_TYPE_INTERRUPT_GATE);
    // Add more as needed, e.g., breakpoint (int3), NMI, etc.

    gIDTR.Limit = sizeof(gIDT) - 1;
    gIDTR.Base  = (u64)&gIDT;

    asm volatile ("lidt %0" : : "m"(gIDTR));

}