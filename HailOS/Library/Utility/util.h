#pragma once

#include "commonlib.h"
#include "typelib.h"

NORETURN void HaltProcessor(void);
NORETURN void Panic(u32, u32, u32);
NORETURN void Reboot(void);

void Wait(int);

extern HOSstatus gLastStatus;