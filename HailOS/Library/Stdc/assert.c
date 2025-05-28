#include "assert.h"
#include "typelib.h"
#include "stdio.h"
#include "print.h"
#include "status.h"
#include "stdcolor.h"
#include "util.h"

void internal_impl_assert(const char* Expression, u32 Line)
{
    SetBackgroundColor(COLOR_GRAY);
    FillScreenWithBackgroundColor();
    SetCursorPos(COORD(0, 0));
    puts("Assertion Failed!");
    HaltProcessor();
}