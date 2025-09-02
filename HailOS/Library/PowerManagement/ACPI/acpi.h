/*
    ACPI定義・操作
    作成日 2025-09-02
*/

#include "basetype.h"
#include "common.h"

typedef struct
{
    char Signature[4];
    u32 Length;
    u8 Revision;
    u8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRev;
} PACKED acpi_sdt_header_t;

typedef struct
{
    char Signature[8]; //RSD PTR
    u8 Checksum;
    char OEMID[8];
    u8 Revision;
    u32 RsdtAddress;
    u32 Length;
    u64 XsdtAddress;
    u8 ExtendedChecksum;
    u8 Reserved[3];
} PACKED acpi_rsdp_t;

typedef struct
{
    u8 AddressSpaceID;
    u8 RegisterBitWidth;
    u8 RegisterBitOffset;
    u8 AccessSize;
    u64 Address;
} PACKED acpi_gas_t;

typedef struct
{
    acpi_sdt_header_t Hdr;
    u32 FirmwareCtrl;
    u32 Dsdt;
    u8 Reserved1;
    u8 PreferredPowerProfile;
    u16 SciInterrupt;
    u32 SmiCommandPort;
    u8 AcpiEnable;
    u8 AcpiDisable;
    u8 S4BiosReq;
    u8 PstateCtrl;
    u32 Pm1aEventBlock;
    u32 Pm1bEventBlock;
    u32 Pm1aCtrlBlock;
    u32 Pm1bCtrlBlock;
    u32 Pm2CtrlBlock;
    u32 PmTimerBlock;
    u32 Gpe0Block;
    u32 Gpe1Block;
    u8 Gpe1BlockLen;
    u8 Reserved2[3];
    u32 Reserved3;
    u32 Reserved4;
    u64 X_FirmwareCtrl;
    u64 X_Dsdt;
    acpi_gas_t X_Pm1aEventBlock;
    acpi_gas_t X_Pm1bEventBlock;
    acpi_gas_t X_Pm1aCtrlBlock;
    acpi_gas_t X_Pm1bCtrlBlock;
} PACKED acpi_fadt_min_t;

#define RSDP_SIGNATURE "RSD PTR "

/// @brief ACPIシャットダウン
NORETURN void ACPIShutdown(void);