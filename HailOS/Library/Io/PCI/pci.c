#include "basetype.h"
#include "pci.h"
#include "io.h"
#include "pci_id.h"
#include "string.h"
#include "system_console.h"

u32 PCIConfigRead32(u8 Bus, u8 Device, u8 Function, u8 Offset)
{
    //先頭ビット = Enable bit（1=有効）
    u32 address = (1u << 31) | ((u32)Bus << 16) | ((u32)Device << 11) | ((u32)Function << 8) | (Offset & 0xFC);
    outl(CONFIG_ADDRESS, address);
    return inl(CONFIG_DATA);
}

void EnumeratePCIDevicesAndPrint(void)
{
    for(u16 bus=0; bus<256; bus++)
    {
        for(u8 device=0; device<32; device++)
        {
            for(u8 function=0; function<8; function++)
            {
                u32 id = PCIConfigRead32(bus,device, function, 0);
                u16 vendor_id = id & 0xFFFF;
                if(vendor_id == PCI_VENDOR_ID_INVALID)
                {
                    // 存在しない・無効
                    continue;
                }

                u16 device_id = (id >> 16) & 0xFFFF;

                puts("PCI device found: vendor=");
                puts(utos(vendor_id));
                puts(" device=");
                puts(utos(device_id));
                puts(" bus=");
                puts(utos(bus));
                puts(" dev=");
                puts(utos(device));
                puts(" func=");
                puts(utos(function));
                puts("\r\n");
            }
        }
    }
}

const char* PCIVendorIDToString(u16 Vendor)
{
    switch(Vendor)
    {
        case PCI_VENDOR_ID_AMD:
            return "AMD";
        case PCI_VENDOR_ID_ASUS:
            return "ASUS";
        case PCI_VENDOR_ID_DELL:
            return "DELL";
        case PCI_VENDOR_ID_INNOTEK:
            return "INNOTEK";
        case PCI_VENDOR_ID_INTEL:
            return "INTEL";
        case PCI_VENDOR_ID_MARVELL:
            return "MARVELL";
        case PCI_VENDOR_ID_MICRON:
            return "MICRON";
        case PCI_VENDOR_ID_MICROSOFT:
            return "MICROSOFT";
        case PCI_VENDOR_ID_MSI:
            return "MSI";
        case PCI_VENDOR_ID_NVIDIA:
            return "NVIDIA";
        case PCI_VENDOR_ID_ORACLE:
            return "ORACLE";
        case PCI_VENDOR_ID_REALTEK:
            return "REALTEK";
        case PCI_VENDOR_ID_SEAGATE:
            return "SEAGATE";
        case PCI_VENDOR_ID_VMWARE:
            return "VMWARE";
        case PCI_VENDOR_ID_INVALID:
            return "Invalid PCI Vendor ID";
        default:
            return NULL;
    }
}