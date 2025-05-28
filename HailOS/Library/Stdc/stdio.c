#include "stdio.h"
#include "print.h"
#include "stdcolor.h"

void puts(const char* Str)
{
    PutString(Str, COLOR_WHITE);
    PutString("\r\n", COLOR_WHITE);
}