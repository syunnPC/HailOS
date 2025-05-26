#include "timeinfo.h"
#include "boot.h"
#include "typelib.h"

hwclockinfo_t* gHwClockInfo;

u64 ReadTsc(void)
{
    u32 lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi<<32)  | lo;
}

u64 GetCurrentUnixTime(void)
{
    u64 currentTsc = ReadTsc();
    u64 delta = currentTsc - gHwClockInfo->InitialTsc;
    u64 deltaSecounds = delta / gHwClockInfo->TscFreq;
    return gHwClockInfo->InitialUnixTime + deltaSecounds;
}