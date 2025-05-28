#include "pic.h"
#include "ps2kbd.h"
#include "typelib.h"
#include "iomgr.h"
#include "init.h"
#include "keycode.h"
#include "print.h"
#include "memmgr.h"
#include "string.h"
#include "vga.h"
#include "stdio.h"

volatile u8 KeyBuffer[KEY_BUFFER_SIZE];
volatile boolean ShiftStatus = false;
volatile int offsetWrite = 0, offsetRead = 0;

void KeyboardHandler(void)
{
    u8 ScanCode = inb(PS2_DATA_PORT);

    if(offsetWrite == KEY_BUFFER_SIZE)
    {
        offsetWrite = 0;
        offsetRead = 0;
    }

    KeyBuffer[offsetWrite++] = ScanCode;

    PicSendEoi(IRQ_KEYBOARD);
}

int ReadKeyFromBuffer(u8* Out)
{
    if(offsetWrite == offsetRead)
    {
        return 0;
    }

    if(offsetRead == KEY_BUFFER_SIZE)
    {
        offsetRead = 0;
    }

    *Out = KeyBuffer[offsetRead++];
    return 1;
}

u8 ScanCodeToAscii(u8 ScanCode)
{
    if(ScanCode & 0x80)
    {
        u8 c = ScanCode & 0x7F;
        if(c == LEFT_SHIFT_DOWN || c == RIGHT_SHIFT_DOWN)
        {
            ShiftStatus = false;
        }

        return 0;
    }

    if(ScanCode == LEFT_SHIFT_DOWN || ScanCode == RIGHT_SHIFT_DOWN)
    {
        ShiftStatus = true;
        return 0;
    }

    if(ShiftStatus)
    {
        return ScanCodeToAsciiShift[ScanCode];
    }
    else
    {
        return ScanCodeToAsciiBasic[ScanCode];
    }
}

u8 ReadKeyWithoutEcho(void)
{
    u8 sc;
    while(true)
    {
        if(ReadKeyFromBuffer(&sc))
        {
            sc = ScanCodeToAscii(sc);
            if(sc != 0)
            {
                return sc;
            }
        }
    }
}

u8 ReadKeyWithEcho(rgbcolor_t Color)
{
    u8 sc;
    while(true)
    {
        sc = ReadKeyWithoutEcho();
        if(((u8)sc >= 0x20 || (u8)sc <= 0x7E))
        {
            break;
        }
    }

    coordinate2D_t loc = GetCurrentCursorPos();
    if(loc.X > gGraphicInfo->HorizontalResolution - 8)
    {
        loc.X = 0;
        loc.Y += 16;
    }

    PrintCharToBuffer(sc, loc, Color);
    if(loc.X > gGraphicInfo->HorizontalResolution - 8)
    {
        loc.X = 0;
        loc.Y += 16;
    }
    else 
    {
        loc.X += 8;
    }

    SetCursorPos(loc);
    DrawBufferContextToFrameBuffer();
    return sc;
}

u8* ReadKeysWithoutEcho(size_t BufferSize)
{
    u8* result = KernelAlloc(BufferSize+1);
    if(result == NULL)
    {
        return NULL;
    }

    size_t readSize=0;
    u8 buf;
    while(true)
    {
        if(ReadKeyFromBuffer(&buf))
        {
            if(buf == 0x1C)
            {
                return result;
            }

            if(readSize == BufferSize)
            {
                return result;
            }

            result[readSize++] = buf;
        }
    }
}

u8* ReadKeysWithEcho(size_t BufferSize, rgbcolor_t Color)
{
    u8* result = KernelAlloc(BufferSize+1);
    if(result == NULL)
    {
        return NULL;
    }

    size_t readSize=0;
    u8 buf;
    while(true)
    {
        buf = ReadKeyWithEcho(Color);
        if(buf != '\n' && buf != '\b' && ((u8)buf >= 0x20 && (u8)buf <= 0x7E))
        {
            if(readSize > BufferSize)
            {
                continue;
            }

            result[readSize++] = buf;
            continue;
        }
        else if(buf == '\b')
        {
            if(readSize != 0)
            {
                DeletePreviousCharacter();
                readSize -= 1;
            }
            continue;
        }
        else if(buf != '\n')
        {
            continue;
        }
        else
        {
            puts("");
            DrawBufferContextToFrameBuffer();
            return result;
        }
    }
}