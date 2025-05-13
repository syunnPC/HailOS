#pragma once

#include "vgatypes.h"

extern graphic_info_t gGraphicInfo;

EFI_STATUS InitializeGraphics(void);
framebuffer_color_t ConvertColor(rgbcolor_t);
HOSstatus DrawPixel(coordinate2D_t, rgbcolor_t);