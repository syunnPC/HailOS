#include "system_console.h"
#include "vga.h"
#include "vgatype.h"
#include "basic_font.h"
#include "string.h"
#include "status.h" 
#include "keycode.h"
#include "ps2kbdapi.h"
#include "memutil.h"
#include "file.h"
#include "bitmap.h"

/*
    注意:フレームバッファへループしてアクセスする時はy->xの順でループすること
*/

static coordinate_t sCursorPos = {0, 0};

/// @brief 文字の描写に改行が必要かを確かめる
/// @return 改行が必要ならtrue, そうでないならfalse
static inline bool IsNewLineRequired(void)
{
    rectangle_t screen_size = GetScreenResolution();
    if(sCursorPos.X > screen_size.Width - FONT_WIDTH)
    {
        return true;
    }
    return false;
}

/// @brief スクロールが必要か確かめる
/// @return 必要ならtrue, そうでないならfalse
static inline bool IsScrollRequired(void)
{
    rectangle_t screen_size = GetScreenResolution();
    if(sCursorPos.Y > screen_size.Height - FONT_HEIGHT)
    {
        return true;
    }
    return false;
}

/// @brief 描写可能な文字であるかを判定
/// @param Ch 文字
/// @return 可能ならtrue, そうでなければfalse
static inline bool IsVisibleAsciiChar(char Ch)
{
    if(Ch > 0x19 && Ch < 0x7f)
    {
        return true;
    }
    return false;
}

/// @brief 文字をバッファに描画（フレームバッファへ転送しない）
/// @param Ch 文字
/// @param Color 文字色
static void PrintCharToBuffer(char Ch, rgb_t Color)
{
    if(IsVisibleAsciiChar(Ch))
    {
        const u8* font = ascii_font8x16[(int)Ch];
        if(IsNewLineRequired())
        {   
            sCursorPos.X = 0;
            sCursorPos.Y += FONT_HEIGHT;
        }
        if(IsScrollRequired())
        {
            sCursorPos.X=0;
            while(IsScrollRequired())
            {
                sCursorPos.Y -= FONT_HEIGHT;
                Scroll(1);
            }
        }
        if(Ch == ' ')
        {
        sCursorPos.X += FONT_WIDTH;
        return;
        }
        for(int i=0; i<FONT_HEIGHT; i++)
        {
            u8 line = font[i];
            for(int k=0; k<FONT_WIDTH; k++)
            {
                if((line & (1 << (7-k))))
                {
                    DrawPixelToBuffer(COORD(sCursorPos.X+k, sCursorPos.Y+i), Color);
                }
            }
        }
        sCursorPos.X += FONT_WIDTH;
    }
    else
    {
        switch(Ch)
        {
            case '\n':
                sCursorPos.Y += FONT_HEIGHT;
                while(IsScrollRequired())
                {
                    Scroll(1);
                    sCursorPos.Y -= FONT_HEIGHT;
                }
                return;
            case '\r':
                sCursorPos.X = 0;
                return;
            case '\b':
                DeleteChar();
                return;
            case '\t':
                if(!(sCursorPos.X + 4*FONT_WIDTH >= GetScreenResolution().Width))
                {
                    sCursorPos.X += 4*FONT_WIDTH;
                }
                return;
            default:
                return;
        }
    }
}

void PrintChar(char Ch, rgb_t Color)
{
    PrintCharToBuffer(Ch, Color);
    DrawBufferContentsToFrameBuffer();
}

void PrintString(const char* Str, rgb_t Color)
{
    for(size_t i=0; i<strlen(Str); i++)
    {
        PrintCharToBuffer(Str[i], Color);
    }
    DrawBufferContentsToFrameBuffer();
}

void Scroll(u32 Line)
{
    if(Line == 0)
    {
        return;
    }
    if(Line >= GetScreenResolution().Height/FONT_HEIGHT)
    {
        ClearBuffer();
        FillScreenWithBackgroundColor();
        sCursorPos = COORD(0, 0);
        return;
    }
    ShiftBufferContents(FONT_HEIGHT, VERTICAL_UP);
    FillScreenWithBackgroundColor();
    DrawBufferContentsToFrameBuffer();
}

coordinate_t SetCursorPos(coordinate_t Location)
{
    coordinate_t old = sCursorPos;
    sCursorPos = Location;
    return old;
}

coordinate_t GetCursorPos(void)
{
    return sCursorPos;
}

void DeleteCharOnBuffer(void)
{
    if(sCursorPos.X > FONT_WIDTH)
    {
        sCursorPos.X -= FONT_WIDTH;
    }
    else
    {
        if(sCursorPos.Y >= FONT_HEIGHT)
        {
            sCursorPos.X = ((GetScreenResolution().Width/FONT_WIDTH)*FONT_WIDTH)-FONT_WIDTH;
            sCursorPos.Y -= FONT_HEIGHT;
        }
        else
        {
            return;
        }
    }

    for(u32 y=0; y<FONT_HEIGHT; y++)
    {
        for(u32 x=0; x<FONT_WIDTH; x++)
        {
            SetEmptyPixelOnBuffer(COORD(sCursorPos.X+x, sCursorPos.Y+y));
        }
    }
}

void DeleteChar(void)
{
    DeleteCharOnBuffer();
    FillScreenWithBackgroundColor();
    DrawBufferContentsToFrameBuffer();
}

char ReadKeyWithEcho(rgb_t Color)
{
    u8 sc;
    while(true)
    {
        if(ReadKeyFromBuffer(&sc))
        {
            sc = ScancodeToAscii(sc);
            if(sc != 0)
            {
                PrintChar(sc, Color);
                return sc;
            }
        }
    }
}

size_t ReadInputWithEcho(char* Buffer, size_t BufferSize, rgb_t Color, bool NewLine)
{
    if(Buffer == NULL && BufferSize != 0)
    {
        return 0;
    }
    size_t read_size = 0;
    u8 sc;
    char ch;
    FillMemory(Buffer, BufferSize, 0);
    while(read_size < BufferSize - 1)
    {
        if(ReadKeyFromBuffer(&sc))
        {
            if(sc == ENTER_KEY)
            {
                if(NewLine)
                {
                    PrintString("\r\n", COLOR_WHITE);
                }
                Buffer[read_size] = '\0';
                return read_size;
            }
            else if(sc == 0x0E)
            {
                if(read_size > 0)
                {
                    read_size--;
                    Buffer[read_size] = '\0';
                    DeleteChar();
                }
            }
            else
            {
                ch = ScancodeToAscii(sc);
                if(ch != 0)
                {
                    Buffer[read_size++] = ch;
                    PrintChar(ch, Color);
                }
            }
        }
    }
    Buffer[read_size] = '\0';
    return read_size;
}

void DrawBitmapInline(const char* FileName)
{
    file_object_t object;
    rectangle_t pic_size = GetPictureSize(FileName);
    HOSstatus status = OpenFile(FileName, &object);
    if(HOS_ERROR(status))
    {
        return;
    }
    u64 screen_height = GetScreenResolution().Height;
    if(screen_height - sCursorPos.Y < pic_size.Height)
    {
        ShiftBufferContents((pic_size.Height - (screen_height - sCursorPos.Y)) + FONT_HEIGHT, VERTICAL_UP);
        FillScreenWithBackgroundColor();
        SetCursorPos(COORD(0, (screen_height - pic_size.Height) - FONT_HEIGHT));
        DrawBufferContentsToFrameBuffer();
    }
    DrawBitmap(FileName, GetCursorPos(), NULL);
    SetCursorPos(COORD(0, GetCursorPos().Y+pic_size.Height));
    CloseFile(&object);
}