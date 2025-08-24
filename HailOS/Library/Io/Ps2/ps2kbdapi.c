#include "ps2kbdapi.h"
#include "keycode.h"
#include "basetype.h"
#include "system_console.h"
#include "memutil.h"
#include "ps2kbd.h"

#define KEY_BUFFER_SIZE 256

static bool sShiftStatus = false;

bool ReadKeyFromBuffer(u8* Out)
{
    if(gOffsetWrite == gOffsetRead)
    {
        return false;
    }

    if(gOffsetRead == KEY_BUFFER_SIZE)
    {
        gOffsetRead = 0;
    }

    *Out = gKeyBuffer[gOffsetRead++];
    return true;
}

char ScancodeToAscii(u8 Scancode)
{
    if(Scancode & 0x80)
    {
        char c = Scancode & 0x7F;
        if(c == LEFT_SHIFT_DOWN || c==RIGHT_SHIFT_DOWN)
        {
            sShiftStatus = true;
        }
        return 0;
    }
    if(Scancode == LEFT_SHIFT_DOWN || Scancode == RIGHT_SHIFT_DOWN)
    {
        sShiftStatus = true;
        return 0;
    }
    if(sShiftStatus)
    {
        return ScancodeToAsciiShift[Scancode];
    }
    else
    {
        return ScancodeToAsciiBasic[Scancode];
    }
}

char ReadKey(void)
{
    u8 sc;
    while(true)
    {
        if(ReadKeyFromBuffer(&sc))
        {
            sc = ScancodeToAscii(sc);
            if(sc != 0)
            {
                return sc;
            }
        }
    }
}

size_t ReadInput(char* Buffer, size_t BufferSize)
{
    if(Buffer == NULL && BufferSize != 0)
    {
        return 0;
    }
    size_t read_size = 0;
    u8 sc;
    FillMemory(Buffer, BufferSize, 0);
    while(true)
    {
        if(ReadKeyFromBuffer(&sc))
        {
            if(sc == ENTER_KEY)
            {
                return read_size;
            }
            if(read_size == BufferSize - 1)
            {
                return read_size;
            }

            Buffer[read_size++] = ScancodeToAscii(sc);
        }
    }
}