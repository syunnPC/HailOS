#pragma once

#include "typelib.h"

#define PIC_EOI 0x20
#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

#define ICW1_INIT     0x10
#define ICW1_ICW4     0x01
#define ICW4_8086     0x01

#define PIC_MASTER_ISR_OFFSET 0x20
#define PIC_SLAVE_ISR_OFFSET 0x28

void IoWait(void);
void RemapPic(u32, u32);
void PicUnmaskIrq(u8);
void PicSendEoi(u8);