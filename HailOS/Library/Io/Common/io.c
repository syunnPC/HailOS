#include "io.h"
#include "basetype.h"

void outb(u16 Port, u8 Value)
{
    asm volatile
    (
        "outb %0, %1"
        :
        : "a"(Value), "Nd"(Port)
    );
}

void outl(u16 Port, u32 Value)
{
    asm volatile
    (
        "outl %0, %1"
        :
        : "a"(Value), "Nd"(Port)
    );
}

u8 inb(u16 Port)
{
    u8 result;
    asm volatile
    (
        "inb %1, %0"
        : "=a"(result)
        : "Nd"(Port)
    );
    return result;
}

u16 inw(u16 Port)
{
    u16 result;
    asm volatile
    (
        "inw %1, %0"
        : "=a"(result)
        : "Nd"(Port)
    );
    return result;
}

u32 inl(u16 Port)
{
    u32 result;
    asm volatile
    (
        "inl %1, %0"
        : "=a"(result)
        : "Nd"(Port)
    );
}