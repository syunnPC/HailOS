#pragma once

#include "vga.h"

#define RGB_COLOR(r, g, b) = (rgbcolor_t){.Red = r, .Blue = g, .Green = b}
#define COLOR_RED  (rgbcolor_t){.Red = 255, .Blue = 0, .Green = 0}
#define COLOR_BLUE (rgbcolor_t){.Red = 0, .Blue = 255, .Green = 0}
#define COLOR_LIME (rgbcolor_t){.Red = 0, .Blue = 0, .Green = 255}
#define COLOR_BLACK (rgbcolor_t){.Red = 0, .Blue = 0, .Green = 0}
#define COLOR_WHITE (rgbcolor_t){.Red = 255, .Blue = 255, .Green = 255}
#define COLOR_SILVER (rgbcolor_t){.Red = 192, .Blue = 192, .Green = 192}
#define COLOR_GRAY (rgbcolor_t){.Red = 128, .Blue = 128, .Green = 128}
#define COLOR_MAROON (rgbcolor_t){.Red = 128, .Blue = 0, .Green = 0}
#define COLOR_YELLOW (rgbcolor_t){.Red = 255, .Green = 255, .Blue = 0}
#define COLOR_OLIVE (rgbcolor_t){.Red = 128, .Green = 128, .Blue = 0}
#define COLOR_GREEN (rgbcolor_t){.Red = 0, .Green = 128, .Blue = 0}
#define COLOR_AQUA (rgbcolor_t){.Red = 0, .Blue = 255, .Green = 255}
#define COLOR_TEAL (rgbcolor_t){.Red = 0, .Blue = 128, .Green = 128}
#define COLOR_NAVY (rgbcolor_t){.Red = 0, .Blue = 128, .Green = 0}
#define COLOR_FUCHSIA (rgbcolor_t){.Red = 255, .Blue = 0, .Green = 255}
#define COLOR_PURPLE (rgbcolor_t){.Red = 128, .Blue = 0, .Green = 128}