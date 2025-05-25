#include "assert.h"
#include "typelib.h"
#include "print.h"
#include "status.h"

void internal_impl_assert(const char* Expression, u32 Line)
{
    Fill((rgbcolor_t){.Red = 255, .Blue = 255, .Green = 255});
    PrintString("Assertion failed!", (coordinate2D_t){.X = 30, .Y = 30}, (rgbcolor_t){.Red = 0, .Blue = 0, .Green = 0});
    PANIC(STATUS_ASSERTION_FAILED, Line);
}