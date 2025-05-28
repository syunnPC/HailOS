#pragma once

#include "typelib.h"
#include "vga.h"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64

#define KEYCODE_MASK_RELEASE 0x80

#define KEY_BUFFER_SIZE 256

extern volatile u8 KeyBuffer[KEY_BUFFER_SIZE];
extern volatile int KeyBufferHead, KeyBufferTail;

void KeyboardHandler(void);
int ReadKeyFromBuffer(u8*);
u8 ScanCodeToAscii(u8);
u8 ReadKeyWithoutEcho(void);
u8* ReadKeysWithEcho(size_t, rgbcolor_t);
u8* ReadKeysWithoutEcho(size_t);
