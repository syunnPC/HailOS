#include "assert.h"
#include "typelib.h"
#include "print.h"
#include "status.h"
#include "stdcolor.h"

void internal_impl_assert(const char* Expression, u32 Line)
{
    SetBackgroundColor(COLOR_WHITE);
    FillScreenWithBackgroundColor();
    PrintStringInAutoFormat("Assertion failed!", COLOR_WHITE);
    PANIC(STATUS_ASSERTION_FAILED, Line);
}