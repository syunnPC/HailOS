/*
    PCI-SIG PCI Vendor ID定義
    作成日 2025-08-24
*/

#pragma once

#include "basetype.h"

#define PCI_VENDOR_ID_INVALID 0xFFFF
#define PCI_VENDOR_ID_INTEL 0x8086
#define PCI_VENDOR_ID_AMD 0x1022
#define PCI_VENDOR_ID_NVIDIA 0x10DE
#define PCI_VENDOR_ID_REALTEK 0x10EC
#define PCI_VENDOR_ID_MICROSOFT 0x1414
#define PCI_VENDOR_ID_MARVELL 0x1DCA
#define PCI_VENDOR_ID_MSI 0x1462
#define PCI_VENDOR_ID_MICRON 0x1344
#define PCI_VENDOR_ID_SEAGATE 0x1BB1
#define PCI_VENDOR_ID_ASUS 0x1043
#define PCI_VENDOR_ID_DELL 0x1028
#define PCI_VENDOR_ID_VMWARE 0x15AD
#define PCI_VENDOR_ID_ORACLE 0x108E
#define PCI_VENDOR_ID_INNOTEK 0x80EE

/// @brief PCIベンダーIDから文字列へ変換
/// @param Vendor ベンダーID
/// @return 存在しないベンダーならNULL
const char* PCIVendorIDToString(u16);