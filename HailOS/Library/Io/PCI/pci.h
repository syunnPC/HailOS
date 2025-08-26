/*
    PCIライブラリ
    作成日 2025-08-24
*/

#pragma once

#include "basetype.h"
#include "common.h"
#include "io.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_SUBCLASS_SATA 0x06
#define PCI_PROGIF_AHCI 0x01

/*
    Config Spaceデータ
    Bus : 0~255
    Device : 0~31
    Function : 0~7
    Reg : 0~63 { 256byteを32bitずつ取得 }
*/

/// @brief PCI Config Spaceを読む
/// @param Bus Bus番号
/// @param Device Device番号
/// @param Function Function番号
/// @param Offset 
/// @return PCI Config Space(32ビットずつ)
u32 PCIConfigRead32(u8, u8, u8, u8);

/// @brief PCIデバイスをスキャン、列挙して情報を出力
void EnumeratePCIDevicesAndPrint(void);