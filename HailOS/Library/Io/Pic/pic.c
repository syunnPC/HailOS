#include "pic.h"
#include "basetype.h"
#include "io.h"

void IoWait(void)
{
    asm volatile
    (
        "outb %%al, $0x80"
        :
        : "a"(0)
    );
}

void RemapPic(u32 OffsetMaster, u32 OffsetSlave)
{
    u8 a1, a2;
    a1 = inb(PIC_MASTER_DATA);
    a2 = inb(PIC_SLAVE_DATA);
    outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_ICW4);
    IoWait();
    outb(PIC_SLAVE_CMD, ICW1_INIT | ICW1_ICW4);
    IoWait();

    outb(PIC_MASTER_DATA, OffsetMaster);
    IoWait();
    outb(PIC_SLAVE_DATA, OffsetSlave);
    IoWait();

    outb(PIC_MASTER_DATA, 0x04);
    IoWait();
    outb(PIC_SLAVE_DATA, 0x02);
    IoWait();

    outb(PIC_MASTER_DATA, ICW4_8086);
    IoWait();
    outb(PIC_SLAVE_DATA, ICW4_8086);
    IoWait();

    outb(PIC_MASTER_DATA, a1);
    outb(PIC_SLAVE_DATA, a2);
}

void PicUnmaskIrq(u8 Irq)
{
    u16 port;
    u8 value;
    if(Irq < 8)
    {
        port = PIC_MASTER_DATA;
    }
    else
    {
        port = PIC_SLAVE_DATA;
        Irq -= 8;
    }

    value = inb(port) & ~(1<<Irq);
    outb(port, value);
}

void PicSendEoi(u8 Irq)
{
    if(Irq >= 8)
    {
        outb(PIC_SLAVE_CMD, PIC_EOI);
    }
    outb(PIC_MASTER_CMD, PIC_EOI);
}