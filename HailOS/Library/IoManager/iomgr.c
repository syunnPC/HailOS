#include "iomgr.h"
#include "typelib.h"

void outb(u16 Port, u8 Value)
{
    asm volatile ("outb %0, %1" : : "a"(Value), "Nd"(Port));
}

u8 inb(u16 Port)
{
    u8 Ret;
    __asm__ volatile ("inb %1, %0" : "=a"(Ret) : "Nd"(Port));
    return Ret;
}

u16 inw(u16 Port)
{
    u16 Ret;
    __asm__ volatile ("inw %1, %0" : "=a"(Ret) : "Nd"(Port));
    return Ret;
}