#include "init.h"
#include "Common/commonlib.h"
#include "Utility/util.h"

INTERRUPT void DefaultHandler(UNUSED void* Frame)
{
    HaltProcessor();
}

INTERRUPT void IsrDivideError(UNUSED void* Frame)
{
    HaltProcessor();
}

INTERRUPT void IsrInvalidOpcode(UNUSED void* Frame)
{
    HaltProcessor();
}

INTERRUPT void IsrDoubleFault(UNUSED void* Frame, UNUSED u64 ErrorCode)
{
    HaltProcessor();
}

INTERRUPT void IsrGPF(UNUSED void* Frame, UNUSED u64 ErrorCode)
{
    HaltProcessor();
}

INTERRUPT void IsrPageFault(UNUSED void* Frame, UNUSED u64 ErrorCode)
{
    u64 Addr;
    asm volatile("mov %%cr2, %0" : "=r"(Addr));
    HaltProcessor();
}