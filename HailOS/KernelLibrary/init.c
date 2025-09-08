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

void InitIDT()
{
    // Initialize all entries to a default handler first
    for (int i = 0; i < IDT_ENTRIES; i++)
    {
        SetIDTEntry(i, DefaultHandler, IDT_TYPE_INTERRUPT_GATE);
    }
    // Set specific handlers for common exceptions
    /*
    SetIDTEntry(0,  IsrDivideError,   IDT_TYPE_INTERRUPT_GATE); // #DE
    SetIDTEntry(6,  IsrInvalidOpcode, IDT_TYPE_INTERRUPT_GATE); // #UD
    SetIDTEntry(8,  IsrDoubleFault,   IDT_TYPE_INTERRUPT_GATE); // #DF (must be interrupt gate, error code pushed)
    SetIDTEntry(13, IsrGPF,           IDT_TYPE_INTERRUPT_GATE); // #GP (error code pushed)
    SetIDTEntry(14, IsrPageFault,     IDT_TYPE_INTERRUPT_GATE); // #PF (error code pushed)
    */

    SetIDTEntry(0, Handler0, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(1, Handler1, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(2, Handler2, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(3, Handler3, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(4, Handler4, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(5, Handler5, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(6, Handler6, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(7, Handler7, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(8, Handler8, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(9, Handler9, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(10, Handler10, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(11, Handler11, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(12, Handler12, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(13, Handler13, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(14, Handler14, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(15, Handler15, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(16, Handler16, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(17, Handler17, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(18, Handler18, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(19, Handler19, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(20, Handler20, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(21, Handler21, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(22, Handler22, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(23, Handler23, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(24, Handler24, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(25, Handler25, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(26, Handler26, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(27, Handler27, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(28, Handler28, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(29, Handler29, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(30, Handler30, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(31, Handler31, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(32, Handler32, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(33, Handler33, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(34, Handler34, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(35, Handler35, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(36, Handler36, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(37, Handler37, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(38, Handler38, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(39, Handler39, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(40, Handler40, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(41, Handler41, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(42, Handler42, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(43, Handler43, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(44, Handler44, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(45, Handler45, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(46, Handler46, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(47, Handler47, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(48, Handler48, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(49, Handler49, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(50, Handler50, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(51, Handler51, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(52, Handler52, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(53, Handler53, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(54, Handler54, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(55, Handler55, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(56, Handler56, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(57, Handler57, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(58, Handler58, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(59, Handler59, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(60, Handler60, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(61, Handler61, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(62, Handler62, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(63, Handler63, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(64, Handler64, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(65, Handler65, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(66, Handler66, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(67, Handler67, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(68, Handler68, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(69, Handler69, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(70, Handler70, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(71, Handler71, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(72, Handler72, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(73, Handler73, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(74, Handler74, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(75, Handler75, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(76, Handler76, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(77, Handler77, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(78, Handler78, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(79, Handler79, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(80, Handler80, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(81, Handler81, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(82, Handler82, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(83, Handler83, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(84, Handler84, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(85, Handler85, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(86, Handler86, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(87, Handler87, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(88, Handler88, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(89, Handler89, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(90, Handler90, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(91, Handler91, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(92, Handler92, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(93, Handler93, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(94, Handler94, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(95, Handler95, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(96, Handler96, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(97, Handler97, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(98, Handler98, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(99, Handler99, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(100, Handler100, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(101, Handler101, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(102, Handler102, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(103, Handler103, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(104, Handler104, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(105, Handler105, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(106, Handler106, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(107, Handler107, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(108, Handler108, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(109, Handler109, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(110, Handler110, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(111, Handler111, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(112, Handler112, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(113, Handler113, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(114, Handler114, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(115, Handler115, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(116, Handler116, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(117, Handler117, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(118, Handler118, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(119, Handler119, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(120, Handler120, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(121, Handler121, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(122, Handler122, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(123, Handler123, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(124, Handler124, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(125, Handler125, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(126, Handler126, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(127, Handler127, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(128, Handler128, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(129, Handler129, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(130, Handler130, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(131, Handler131, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(132, Handler132, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(133, Handler133, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(134, Handler134, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(135, Handler135, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(136, Handler136, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(137, Handler137, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(138, Handler138, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(139, Handler139, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(140, Handler140, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(141, Handler141, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(142, Handler142, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(143, Handler143, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(144, Handler144, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(145, Handler145, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(146, Handler146, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(147, Handler147, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(148, Handler148, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(149, Handler149, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(150, Handler150, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(151, Handler151, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(152, Handler152, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(153, Handler153, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(154, Handler154, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(155, Handler155, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(156, Handler156, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(157, Handler157, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(158, Handler158, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(159, Handler159, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(160, Handler160, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(161, Handler161, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(162, Handler162, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(163, Handler163, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(164, Handler164, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(165, Handler165, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(166, Handler166, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(167, Handler167, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(168, Handler168, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(169, Handler169, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(170, Handler170, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(171, Handler171, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(172, Handler172, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(173, Handler173, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(174, Handler174, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(175, Handler175, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(176, Handler176, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(177, Handler177, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(178, Handler178, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(179, Handler179, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(180, Handler180, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(181, Handler181, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(182, Handler182, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(183, Handler183, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(184, Handler184, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(185, Handler185, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(186, Handler186, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(187, Handler187, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(188, Handler188, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(189, Handler189, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(190, Handler190, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(191, Handler191, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(192, Handler192, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(193, Handler193, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(194, Handler194, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(195, Handler195, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(196, Handler196, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(197, Handler197, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(198, Handler198, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(199, Handler199, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(200, Handler200, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(201, Handler201, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(202, Handler202, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(203, Handler203, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(204, Handler204, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(205, Handler205, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(206, Handler206, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(207, Handler207, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(208, Handler208, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(209, Handler209, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(210, Handler210, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(211, Handler211, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(212, Handler212, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(213, Handler213, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(214, Handler214, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(215, Handler215, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(216, Handler216, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(217, Handler217, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(218, Handler218, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(219, Handler219, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(220, Handler220, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(221, Handler221, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(222, Handler222, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(223, Handler223, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(224, Handler224, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(225, Handler225, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(226, Handler226, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(227, Handler227, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(228, Handler228, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(229, Handler229, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(230, Handler230, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(231, Handler231, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(232, Handler232, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(233, Handler233, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(234, Handler234, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(235, Handler235, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(236, Handler236, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(237, Handler237, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(238, Handler238, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(239, Handler239, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(240, Handler240, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(241, Handler241, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(242, Handler242, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(243, Handler243, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(244, Handler244, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(245, Handler245, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(246, Handler246, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(247, Handler247, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(248, Handler248, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(249, Handler249, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(250, Handler250, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(251, Handler251, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(252, Handler252, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(253, Handler253, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(254, Handler254, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(255, Handler255, IDT_TYPE_INTERRUPT_GATE);

    SetIDTEntry(IRQ_IDT(7), IsrIRQ7, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(IRQ_IDT(IRQ_KEYBOARD), IsrKeyboard, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(IRQ_IDT(IRQ_MOUSE), IsrMouse, IDT_TYPE_INTERRUPT_GATE);
    SetIDTEntry(IRQ_IDT(0), IsrTimer, IDT_TYPE_INTERRUPT_GATE);
    // Add more as needed, e.g., breakpoint (int3), NMI, etc.
    gIDTR.Limit = sizeof(gIDT) - 1;
    gIDTR.Base = (u64)&gIDT;

    asm volatile("lidt %0" : : "m"(gIDTR));
}