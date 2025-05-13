#pragma once

#include "commonlib.h"
#include "vga.h"

void PrintChar(char,coordinate2D_t,rgbcolor_t);
void PrintString(const char*, coordinate2D_t, rgbcolor_t);
void Print(const char*);
void Fill(rgbcolor_t);
void DrawRect(coordinate2D_t, coordinate2D_t, rgbcolor_t);
void DrawBox(coordinate2D_t,coordinate2D_t,rgbcolor_t);