/*
    AHCIデバイス処理
    作成日 2025-08-24
*/

#pragma once

#include "basetype.h"
#include "common.h"

typedef enum
{
    AHCI_DEV_NULL = 0,
    AHCI_DEV_SATA,
    AHCI_DEV_SEMB,
    AHCI_DEV_PM,
    AHCI_DEV_SATAPI
} ahci_device_type_t;

typedef volatile struct
{
    u32 Cap;
    u32 Ghc;
    u32 Is;
    u32 Pi;
    u32 Vs;
    u32 CccCtl;
    u32 CccPts;
    u32 EmLock;
    u32 EmCtl;
    u32 Cap2;
    u32 Bohc;
    u8 Rsv[0xA0 - 0x2C];
    u8 Vendor[0x100 - 0xA0];
} PACKED hba_mem_t;

typedef volatile struct
{
    u32 Clb;
    u32 Clbu;
    u32 Fb;
    u32 Fbu;
    u32 Is;
    u32 Ie;
    u32 Cmd;
    u32 Rsv0;
    u32 Tfd;
    u32 Sig;
    u32 Ssts;
    u32 Sctl;
    u32 Serr;
    u32 Sact;
    u32 Ci;
    u32 Sntf;
    u32 Fbs;
    u32 rsv1[11];
    u32 vendor[4];
} PACKED hba_port_t;

#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101
#define SATA_SIG_SATA 0x00000101

#define AHCI_GHC_HR (1 << 0)
#define AHCI_GHC_IE (1 << 1)
#define AHCI_GHC_AE (1 << 31)

#define HBA_PxCMD_ST (1 << 0)
#define HBA_PxCMD_FRE (1 << 4)
#define HBA_PxCMD_FR (1 << 14)
#define HBA_PxCMD_CR (1 << 15)

#define HBA_PORT(base, i) ((hba_port_t*)((u8*)(base) + 0x100 + (i)*0x80))

typedef struct
{
    u8 Bus;
    u8 Device;
    u8 Function;
    addr_t Abar;
} ahci_pci_info_t;

/// @brief 指定されたPCIデバイスがAHCIコントローラーかを確認
/// @param Bus バス
/// @param Device デバイス
/// @param Function ファンクション
/// @return もしAHCIコントローラーならtrue
bool IsAHCIController(u8, u8, u8);

/// @brief 指定されたAHCIデバイスのBAR5 HBAのベースアドレス（MMIO）を取得
/// @param Bus バス
/// @param Device デバイス
/// @param Function ファンクション
/// @return　もしAHCIデバイスでなければNULL,それ以外ならHBAのベースアドレス
addr_t GetAHCIMMIOAddress(u8, u8, u8);

/// @brief AHCIコントローラーを見つける
/// @param  Out AHCIコントローラーの情報
/// @return もし見つからなければfalse
bool FindAHCIController(ahci_pci_info_t*);

/// @brief AHCIのデバイスを判別し表示
void AHCIProbeAndListPorts(void);