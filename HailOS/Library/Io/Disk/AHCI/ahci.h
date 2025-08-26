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

#define FIS_TYPE_REG_H2D 0x27
#define FIS_TYPE_REG_D2H 0x34

#define ATA_TFD_BSY (1<<7)
#define ATA_TFD_DRQ (1<<3)

#define TFES_BIT (1u<<30)

#define ATA_CMD_IDENTIFY_DEVICE 0xEC

#define HBA_PORT(base, i) ((hba_port_t*)((u8*)(base) + 0x100 + (i)*0x80))

typedef struct
{
    u32 Dba;
    u32 Dbau;
    u32 Rsv0;
    u32 DbcI;
} PACKED hba_prdt_entry_t;

typedef struct
{
    u8 Cfis[64];
    u8 Acmd[16];
    u8 Rsv[48];
    hba_prdt_entry_t Prdt[1];
} PACKED ALIGN(128) hba_cmd_table_t;

typedef struct 
{
    u8 Cfl:5;
    u8 A:1;
    u8 W:1;
    u8 P:1;
    u8 R:1;
    u8 B:1;
    u8 C:1;
    u8 Rsv0:1;
    u8 Pmp:4;
    u16 Prdtl;
    volatile u32 Prdbc;
    u32 Ctba;
    u32 Ctbau;
    u32 Rsv1[4];
} PACKED hba_cmd_header_t;

typedef struct
{
    u8 Dsfis[0x1C];
    u8 Rsv0[0x04];
    u8 Psfis[0x14];
    u8 Rsv1[0x0C];
    u8 Rfis[0x14];
    u8 Rsv2[0x04];
    u8 Sdbfis[0x08];
    u8 Ufis[0x40];
    u8 Rsv3[0x60];
} PACKED ALIGN(256) fis_recv_t;

typedef struct
{
    u8 FisType;
    u8 Pmport:4;
    u8 Rsv0:3;
    u8 C:1;
    u8 Command;
    u8 Featurel;
    u8 Lba0;
    u8 Lba1;
    u8 Lba2;
    u8 Device;
    u8 Lba3;
    u8 Lba4;
    u8 Lba5;
    u8 Featureh;
    u8 Countl;
    u8 Counth;
    u8 Icc;
    u8 Control;
    u8 Rsv1[4];
} PACKED fis_reg_h2d_t;

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

/// @brief SATAディスクを初期化 
void InitSATA(void);