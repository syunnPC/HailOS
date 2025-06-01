#include "tsc.h"
#include "basetype.h"

u64 ReadTsc(void)
{
    u32 lo, hi;
    asm volatile
    (
        "rdtsc"
        : "=a"(lo), "=d"(hi)
    );
    return ((u64)hi<<32) | lo;
}