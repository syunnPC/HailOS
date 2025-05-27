#include "print.h"
#include "charset.h"
#include "typelib.h"
#include "util.h"
#include "vga.h"

extern graphic_info_t* gGraphicInfo;

static const int DEFAULT_CONSOLE_OFFSET_X = 0;
static const int DEFAULT_CONSOLE_OFFSET_Y = 0;

static coordinate2D_t CurrentPosition = {.X = DEFAULT_CONSOLE_OFFSET_X, .Y = DEFAULT_CONSOLE_OFFSET_Y};

void PrintChar(char Ch, coordinate2D_t Location, rgbcolor_t Color)
{
    if ((u8)Ch < 0x20 || (u8)Ch > 0x7E)
    {
        return;
    }

    const u8* glyph = ascii_font8x16[(u8)Ch];
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

void PrintCharToBuffer(char Ch, coordinate2D_t Location, rgbcolor_t Color)
{
    if ((u8)Ch < 0x20 || (u8)Ch > 0x7E)
    {
        return;
    }

    const u8* glyph = ascii_font8x16[(u8)Ch];
    for (i32 row = 0; row < FONT_HEIGHT; row++)
    {
        u8 bits = glyph[row];
        for (i32 col = 0; col < FONT_WIDTH; col++)
        {
            coordinate2D_t pixel = {Location.X + col, Location.Y + row};
            if (bits & (1 << (7 - col)))
            {
                if (pixel.X >= 0 && pixel.Y >= 0 && pixel.X < (i64)gGraphicInfo->HorizontalResolution && pixel.Y < (i64)gGraphicInfo->VerticalResolution)
                {
                    DrawPixelToBuffer(pixel, Color);
                }
            }
        }
    }
}

void PrintStringInAutoFormat(const char* Str, rgbcolor_t Color)
{
    i64 x = CurrentPosition.X;
    i64 y = CurrentPosition.Y;

    while(*Str)
    {
        char ch = *Str++;
        if(ch == '\r')
        {
            x = DEFAULT_CONSOLE_OFFSET_X;
        }
        else if(ch == '\n')
        {
            y += FONT_HEIGHT;
        }
        else
        {
            coordinate2D_t pos = {x, y};
            PrintChar(ch, pos, Color);
            x+=FONT_WIDTH;
        }

        if (x + FONT_WIDTH > (i64)gGraphicInfo->HorizontalResolution)
        {
            x = DEFAULT_CONSOLE_OFFSET_X;
            y += FONT_HEIGHT;
        }

        if (y + FONT_HEIGHT > (i64)gGraphicInfo->VerticalResolution)
        {
            break;
        }
    }

    CurrentPosition.X = x;
    CurrentPosition.Y = y;
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

    CurrentPosition.X = DEFAULT_CONSOLE_OFFSET_X;
    CurrentPosition.Y = DEFAULT_CONSOLE_OFFSET_Y;
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

void PutString(const char* Str, rgbcolor_t Color)
{
    i64 x = CurrentPosition.X;
    i64 y = CurrentPosition.Y;

    while(*Str)
    {
        char ch = *Str++;
        if(ch == '\r')
        {
            x = DEFAULT_CONSOLE_OFFSET_X;
        }
        else if(ch == '\n')
        {
            y += FONT_HEIGHT;
        }
        else
        {
            coordinate2D_t pos = {x, y};
            PrintCharToBuffer(ch, pos, Color);
            x+=FONT_WIDTH;
        }

        if (x + FONT_WIDTH > (i64)gGraphicInfo->HorizontalResolution)
        {
            x = DEFAULT_CONSOLE_OFFSET_X;
            y += FONT_HEIGHT;
        }

        if (y + FONT_HEIGHT > (i64)gGraphicInfo->VerticalResolution)
        {
            break;
        }
    }

    DrawBufferContextToFrameBuffer();

    CurrentPosition.X = x;
    CurrentPosition.Y = y;
}

void Scroll(size_t ScrollCharactorCount)
{
    FillScreenWithBackgroundColor();
    ShiftBufferContext(ScrollCharactorCount * FONT_HEIGHT, VERTICAL_UP);
    DrawBufferContextToFrameBuffer();
    CurrentPosition.Y -= (ScrollCharactorCount*FONT_HEIGHT > CurrentPosition.Y? CurrentPosition.Y : ScrollCharactorCount*FONT_HEIGHT);
}

void SetCursorPos(coordinate2D_t Location)
{
    CurrentPosition = Location;
}