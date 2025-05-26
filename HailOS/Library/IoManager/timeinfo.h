#pragma once

#include "typelib.h"
#include "boot.h"

extern hwclockinfo_t* gHwClockInfo;

u64 ReadTsc(void);
u64 GetCurrentUnixTime(void);