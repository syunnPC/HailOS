/*
    HALディスク抽象化ライブラリ
    作成日 2025-08-27
*/

#pragma once

#include "basetype.h"
#include "common.h"
#include "ahci.h"

typedef enum
{
    DISK_TYPE_NONE = 0,
    DISK_TYPE_IDE = 1,
    DISK_TYPE_AHCI = 2,
    DISK_TYPE_AHCI_AND_IDE = 3
} disk_type_t;

typedef struct
{
    disk_type_t Type;
    hba_mem_t* Abar;
    int PortIndex;
} disk_t;

/// @brief ディスクから1セクタ読み取り
/// @param  Disk ディスク構造体
/// @param  Lba 読み取るセクタ
/// @param  Buf バッファ
/// @return 正常に読み取れればtrue
bool HALDiskReadSector(disk_t*, u64, u8*);

/// @brief メインディスクから1セクタ読み取り
/// @param  Lba 読み取るセクタ
/// @param  Buf バッファ
/// @return 正常に読み取れればtrue
bool HALMainDiskReadSector(u64, u8*);

/// @brief ディスクを初期化 
/// @return 初期化されたディスクの種類
disk_type_t HALInitDisk(void);