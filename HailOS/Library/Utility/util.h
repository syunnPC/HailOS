#pragma once

#include "Common/commonlib.h"
#include "Common/typelib.h"

NORETURN void HaltProcessor(void);
NORETURN void Panic(u32, u32, u32);
NORETURN void Reboot(void);