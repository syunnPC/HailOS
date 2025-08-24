#include "timer.h"
#include "timedef.h"
#include "basetype.h"
#include "util.h"
#include "tsc.h"

static hw_clockinfo_t* sClockInfo;

void InitTime(hw_clockinfo_t* ClockInfo)
{
    if(ClockInfo == NULL)
    {
        ForceReboot();
    }

    sClockInfo = ClockInfo;
}

time_t GetCurrentUnixTime(void)
{
    u64 current_tsc = ReadTsc();
    u64 delta = current_tsc - sClockInfo->InitialTsc;
    u64 sec = delta/sClockInfo->TscFreq;
    return sClockInfo->InitialUnixTime+sec;
}

u64 GetPerformanceCounter(void)
{
    return ReadTsc();
}

u64 PerformanceCounterTickToMs(u64 Delta)
{
    u64 freq_ms = sClockInfo->TscFreq / 1000;
    return Delta/freq_ms;
}

u64 GetSystemUpTime(void)
{
    return sClockInfo->InitialUnixTime - GetCurrentUnixTime();
}

void Sleep(u64 Ms)
{
    u64 freq_ms = sClockInfo->TscFreq/1000;
    u64 start_tsc = ReadTsc();
    while(ReadTsc() < start_tsc + freq_ms * Ms);
}