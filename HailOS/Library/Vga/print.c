#include "print.h"
#include "charset.h"
#include "Common/typelib.h"
#include "hailkrnl.h"

void PrintChar(char Ch, coordinate2D_t Location, rgbcolor_t Color)
{
    if ((u8)Ch < 0x20 || (u8)Ch > 0x7E)
    {
        return;
    }

    const u8* glyph = font8x16[(u8)Ch];
    for (i32 row = 0; row < FONT_HEIGHT; row++)
    {
        u8 bits = glyph[row];
        for (i32 col = 0; col < FONT_WIDTH; col++)
        {
            if (bits & (1 << (7 - col)))
            {
                coordinate2D_t pixel = {Location.X + col, Location.Y + row};
                if (pixel.X >= 0 && pixel.Y >= 0 && pixel.X < (i64)gGraphicInfo->HorizontalResolution && pixel.Y < (i64)gGraphicInfo->VerticalResolution)
                {
                    DrawPixel(pixel, Color);
                }
            }
        }
    }
}

void PrintString(const char* Str, coordinate2D_t Location, rgbcolor_t Color)
{
    i64 startX = Location.X;
    i64 x = Location.X;
    i64 y = Location.Y;

    while (*Str)
    {
        char ch = *Str++;

        if (ch == '\r')
        {
            x = startX;
        }
        else if (ch == '\n')
        {
            y += FONT_HEIGHT;
        }
        else
        {
            coordinate2D_t pos = {x, y};
            PrintChar(ch, pos, Color);
            x += FONT_WIDTH;

            if (x + FONT_WIDTH > (i64)gGraphicInfo->HorizontalResolution)
            {
                x = startX;
                y += FONT_HEIGHT;
            }
        }

        if (y + FONT_HEIGHT > (i64)gGraphicInfo->VerticalResolution)
        {
            break;
        }
    }
}

void Fill(rgbcolor_t Color)
{
    for (i64 y = 0; y < (i64)gGraphicInfo->VerticalResolution; y++)
    {
        for (i64 x = 0; x < (i64)gGraphicInfo->HorizontalResolution; x++)
        {
            coordinate2D_t pos = { x, y };
            DrawPixel(pos, Color);
        }
    }
}

void DrawRect(coordinate2D_t TopLeft, coordinate2D_t BottomRight, rgbcolor_t Color)
{
    if (TopLeft.X > BottomRight.X)
    {
        i64 tmp = TopLeft.X;
        TopLeft.X = BottomRight.X;
        BottomRight.X = tmp;
    }

    if (TopLeft.Y > BottomRight.Y)
    {
        i64 tmp = TopLeft.Y;
        TopLeft.Y = BottomRight.Y;
        BottomRight.Y = tmp;
    }

    if (TopLeft.X < 0)
    {
        TopLeft.X = 0;
    }

    if (TopLeft.Y < 0)
    {
        TopLeft.Y = 0;
    }

    if (BottomRight.X > (i64)gGraphicInfo->HorizontalResolution)
    {
        BottomRight.X = gGraphicInfo->HorizontalResolution;
    }

    if (BottomRight.Y > (i64)gGraphicInfo->VerticalResolution)
    {
        BottomRight.Y = gGraphicInfo->VerticalResolution;
    }

    for (i64 y = TopLeft.Y; y < BottomRight.Y; y++) 
    {
        for (i64 x = TopLeft.X; x < BottomRight.X; x++) 
        {
            coordinate2D_t pos = { x, y };
            DrawPixel(pos, Color);
        }
    }
}

void DrawBox(coordinate2D_t TopLeft, coordinate2D_t BottomRight, rgbcolor_t Color) 
{
    if (TopLeft.X > BottomRight.X)
    {
        i64 tmp = TopLeft.X;
        TopLeft.X = BottomRight.X;
        BottomRight.X = tmp;
    }

    if (TopLeft.Y > BottomRight.Y)
    {
        i64 tmp = TopLeft.Y;
        TopLeft.Y = BottomRight.Y;
        BottomRight.Y = tmp;
    }

    if (TopLeft.X < 0)
    {
        TopLeft.X = 0;
    }

    if (TopLeft.Y < 0) 
    {
        TopLeft.Y = 0;
    }

    if (BottomRight.X > (i64)gGraphicInfo->HorizontalResolution) 
    {
        BottomRight.X = gGraphicInfo->HorizontalResolution;
    }

    if (BottomRight.Y > (i64)gGraphicInfo->VerticalResolution) 
    {
        BottomRight.Y = gGraphicInfo->VerticalResolution;
    }

    for (i64 x = TopLeft.X; x < BottomRight.X; x++) 
    {
        DrawPixel((coordinate2D_t){ x, TopLeft.Y }, Color);
        DrawPixel((coordinate2D_t){ x, BottomRight.Y - 1 }, Color);
    }

    for (i64 y = TopLeft.Y; y < BottomRight.Y; y++) 
    {
        DrawPixel((coordinate2D_t){ TopLeft.X, y }, Color);
        DrawPixel((coordinate2D_t){ BottomRight.X - 1, y }, Color);
    }
}