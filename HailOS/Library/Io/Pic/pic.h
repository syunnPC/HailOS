/*
    PIC制御用関数・定義
    作成日 2025-05-30
*/

#pragma once 

#include "basetype.h"

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

/// @brief 応答待ち 
void IoWait(void);

/// @brief PICをリマップ 
/// @param OffsetMaster マスターのオフセット
/// @param  OffsetSlave スレーブのオフセット
void RemapPic(u32, u32);

/// @brief IRQをアンマスク
/// @param  Irq アンマスクするIRQ
void PicUnmaskIrq(u8);

/// @brief EOIを送信
/// @param  Irq 終了した割り込みのIRQ
void PicSendEoi(u8);