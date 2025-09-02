#include "ps2mouse.h"
#include "basetype.h"
#include "io.h"
#include "ps2.h"
#include "pic.h"
#include "cursor.h"
#include "vga.h"
#include "vgatype.h"
#include "system_console.h"
#include "string.h"

static mouse_state_t gMouseState = {0};
static u8 sMouseCycle = 0;
static u8 sMouseBytes[4];
static rectangle_t sScreenResolution;

#define IRQ_MOUSE 12

#define PS2_WAIT_LOOP 1000000

void MouseHandler(void)
{
    u8 status = inb(PS2_STATUS_PORT);

    u8 data = inb(PS2_DATA_PORT);

    if(sMouseCycle == 0 && !(data & 0x08))
    {
        PicSendEoi(IRQ_MOUSE);
        return;
    }

    sMouseBytes[sMouseCycle++] = data;

    if(sMouseCycle == 3)
    {
        u8 b = sMouseBytes[0];

        bool sx = b & 0x10;
        bool sy = b & 0x20;

        int dx = (i8)sMouseBytes[1];
        int dy = (i8)sMouseBytes[2];

        gMouseState.X += dx;
        gMouseState.Y -= dy;

        if(gMouseState.X < 0)
        {
            gMouseState.X = 0;
        }

        if(gMouseState.Y < 0)
        {
            gMouseState.Y = 0;
        }

        if(gMouseState.X >= sScreenResolution.Width)
        {
            gMouseState.X = sScreenResolution.Width - 1; 
        }

        if(gMouseState.Y >= sScreenResolution.Height)
        {
            gMouseState.Y = sScreenResolution.Height - 1;
        }

        gMouseState.LeftButton = (b & 0x01) != 0;
        gMouseState.RightButton = (b & 0x02) != 0;
        gMouseState.MiddleButton = (b & 0x04) != 0;

        sMouseCycle = 0;
        
        UpdateCursorBuffer(CURSOR_DEFAULT_COLOR, COORD(gMouseState.X, gMouseState.Y));
    }

    PicSendEoi(IRQ_MOUSE);
}

static bool PS2WaitInputClear(void)
{
    for(int i=0; i<PS2_WAIT_LOOP; i++)
    {
        if((inb(PS2_STATUS_PORT) & 0x02) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool PS2WaitOutputFull(void)
{
    for(int i=0; i<PS2_WAIT_LOOP; i++)
    {
        if(inb(PS2_STATUS_PORT) & 0x01)
        {
            return true;
        }
    }

    return false;
}

static bool WriteCtrlCmd(u8 Cmd)
{
    if(!PS2WaitInputClear())
    {
        return false;
    }

    outb(PS2_STATUS_PORT, Cmd);
    return true;
}

static bool WriteCtrlData(u8 Data)
{
    if(!PS2WaitInputClear())
    {
        return false;
    }

    outb(PS2_DATA_PORT, Data);
    return true;
}

static bool WriteMouse(u8 Data)
{
    if(!PS2WaitInputClear())
    {
        return false;
    }

    outb(PS2_STATUS_PORT, 0xD4);
    if(!PS2WaitInputClear())
    {
        return false;
    }
    outb(PS2_DATA_PORT, Data);
    return true;
}

static bool WaitAck(int Timeout)
{
    for(int i=0; i<Timeout; i++)
    {
        if(PS2WaitOutputFull())
        {
            u8 v = inb(PS2_DATA_PORT);
            if(v == 0xFA)
            {
                return true;
            }
        }
    }

    return false;
}

bool InitMouse(void)
{
    sScreenResolution = GetScreenResolution();

    if(!WriteCtrlCmd(0xA8))
    {
        return false;
    }

    if(!WriteCtrlCmd(0x20))
    {
        return false;
    }

    int wait = PS2_WAIT_LOOP;
    while(wait--)
    {
        if(inb(PS2_STATUS_PORT) & 0x01)
        {
            break;
        }
    }

    if(wait <= 0)
    {
#ifdef DEBUG
        puts("Warning: PS/2 Controller config read timeout.\r\n");
#endif
    }

    u8 status = inb(PS2_DATA_PORT);

    status |= 0x03;

    if(!WriteCtrlCmd(0x60))
    {
        return false;
    }

    if(!WriteCtrlData(status))
    {
        return false;
    }

    while(inb(PS2_STATUS_PORT) & 0x01)
    {
        (void)inb(PS2_DATA_PORT);
    }

    /*
    if(!WriteMouse(0xF6))
    {
        return false;
    }
    */

    if(!WaitAck(PS2_WAIT_LOOP))
    {
        return false;
    }

    if(!WriteMouse(0xF4))
    {
        return false;
    }

    if(!WaitAck(PS2_WAIT_LOOP))
    {
        return false;
    }

    gMouseState.X = 0;
    gMouseState.Y = 0;

    InitCursor();
    UpdateCursorBuffer(CURSOR_DEFAULT_COLOR, COORD(0, 0));

    return true;
}