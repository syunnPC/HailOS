#include "ps2kbd.h"
#include "pic.h"
#include "io.h"
#include "ps2.h"

u8 gKeyBuffer[KEY_BUFFER_SIZE];
size_t gOffsetRead = 0, gOffsetWrite = 0;

void KeyboardHandler(void)
{
    u8 scan_code = inb(PS2_DATA_PORT);
    if(gOffsetWrite == KEY_BUFFER_SIZE)
    {
        gOffsetWrite = 0;
    }

    gKeyBuffer[gOffsetWrite++] = scan_code;

    PicSendEoi(IRQ_KEYBOARD);
}