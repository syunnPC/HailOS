#pragma once

#include "commonlib.h"
#include "vga.h"

void PrintChar(char,coordinate2D_t,rgbcolor_t);
void PrintCharToBuffer(char, coordinate2D_t, rgbcolor_t);
void PrintStringInAutoFormat(const char*, rgbcolor_t);
void Fill(rgbcolor_t);
void DrawRect(coordinate2D_t, coordinate2D_t, rgbcolor_t);
void DrawBox(coordinate2D_t,coordinate2D_t,rgbcolor_t);
void Scroll(size_t);
void PutString(const char*, rgbcolor_t);
void SetCursorPos(coordinate2D_t);
coordinate2D_t GetCurrentCursorPos(void);
void DeletePreviousCharacter(void);

#define PUTS(String) PutString(String, (rgbcolor_t){.Red = 255, .Green = 255, .Blue = 255})
