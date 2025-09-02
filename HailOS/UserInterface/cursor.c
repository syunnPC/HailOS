#include "cursor.h"
#include "vgatype.h"
#include "vga.h"
#include "memutil.h"
#include "system_console.h"

static rectangle_t sScreenSize;
framebuffer_color_t* gCursorDrawBuffer;
static u32 sPpsl;
static framebuffer_color_t* sFBAddr;
bool gCursorState;

#define CALC_PIXEL_OFFSET(x, y) ((y*sPpsl + x)*sizeof(framebuffer_color_t))

#define CURSOR_SIZE_X 8
#define CURSOR_SIZE_Y sizeof(Cursor)

#define IN_RANGE(min, v, max) ((v >= min) && (v <= max))

const u8 Cursor[] = 
{
    0b10000000,0b11000000,0b11100000,0b11110000,0b11111000,0b11111100,0b11111110,0b11111111,0b11111000,0b11011100,0b10011100,0b00001110,0b00001110,0b00000111,0b00000111,0b00000011
};

void InitCursor(void)
{
    gCursorState = true;
    sScreenSize = GetScreenResolution();
    sPpsl = GetPixelPerScanLine();
    sFBAddr = GetFrameBufferRawAddress();
    gCursorDrawBuffer = AllocInitializedMemory(sizeof(framebuffer_color_t)*sScreenSize.Height*sScreenSize.Width, 0x00);
    if(gCursorDrawBuffer == NULL)
    {
        puts("Failed to allocate memory. Cannot start cursor.\r\n");
        return;
    }
}

void UpdateCursorBuffer(rgb_t Color, coordinate_t Pos)
{
    if(gCursorState == false)
    {
        return;
    }

    FillMemory(gCursorDrawBuffer, sizeof(framebuffer_color_t)*sScreenSize.Height*sScreenSize.Width, 0x00);
    u64 draw_x = Pos.X;
    u64 draw_y = Pos.Y;
    for(int i=0; i<CURSOR_SIZE_Y; i++)
    {
        u8 line = Cursor[i];
        for(int k=0; k<CURSOR_SIZE_X; k++)
        {
            if(draw_x + k >= sScreenSize.Width || draw_y + i >= sScreenSize.Height)
            {
                continue;
            }

            if(line & (1 << (7-k)))
            {
                framebuffer_color_t c = ConvertColor(Color);
                addr_t offset = CALC_PIXEL_OFFSET(draw_x + k, draw_y + i);
                framebuffer_color_t* pixel_addr = (framebuffer_color_t*)((addr_t)gCursorDrawBuffer + offset);
                *pixel_addr = c;
            }
        }
    }

    if(IsGraphicAvailable())
    {
        DrawBufferContentsToFrameBuffer();
    }
}

void HideCursor(void)
{
    FillMemory(gCursorDrawBuffer, sizeof(framebuffer_color_t)*sScreenSize.Height*sScreenSize.Width, 0x00);
    DrawBufferContentsToFrameBuffer();
    gCursorState = false;
}

bool GetCursorState(void)
{
    return gCursorState;
}

void ShowCursor(void)
{
    gCursorState = true;
}