#include "ahci.h"
#include "basetype.h"
#include "common.h"
#include "util.h"
#include "status.h"
#include "pci.h"
#include "system_console.h"
#include "string.h"
#include "timer.h"

/// @brief AHCIデバイスの種類を判別
/// @param Port ポート
/// @return AHCIデバイスの種類
static inline ahci_device_type_t CheckType(hba_port_t* Port)
{
    u32 ssts = Port->Ssts;
    u8 ipm = (ssts >> 8) & 0x0F;
    u8 det = ssts & 0x0F;

    if(det != 3)
    {
        return AHCI_DEV_NULL; //未接続
    }

    if(ipm != 1)
    {
        return AHCI_DEV_NULL; //非アクティブ
    }

    switch (Port->Sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_SATA:
            return AHCI_DEV_SATA;
        default:
            return AHCI_DEV_NULL;
    }
}

bool IsAHCIController(u8 Bus, u8 Device, u8 Function)
{
    u32 classcode = PCIConfigRead32(Bus, Device,Function, 0x08);
    u8 prog_if = (classcode >> 8) & 0xFF;
    u8 subclass = (classcode >> 16) & 0xFF;
    u8 class = (classcode >> 24) & 0xFF;

    if(class == PCI_CLASS_MASS_STORAGE && subclass == PCI_SUBCLASS_SATA && prog_if ==  PCI_PROGIF_AHCI)
    {
        return true;
    }
    return false;
}

addr_t GetAHCIMMIOAddress(u8 Bus, u8 Device, u8 Function)
{
    if(!IsAHCIController(Bus, Device, Function))
    {
        return (addr_t)NULL;
    }

    u32 abar_low = PCIConfigRead32(Bus, Device, Function, 0x24);
    return (addr_t)(abar_low & ~0xF);
}

bool FindAHCIController(ahci_pci_info_t* Out)
{
    for(u16 bus = 0; bus<256; bus++)
    {
        for(u8 dev=0; dev<32; dev++)
        {
            for(u8 func=0; func<8; func++)
            {
                u32 id = PCIConfigRead32(bus, dev, func, 0x00);
                u16 vendor = id & 0xFFFF;
                if(vendor == 0xFFFF)
                {
                    continue;
                }

                if(!IsAHCIController(bus, dev, func))
                {
                    continue;
                }

                Out->Bus = bus;
                Out->Device = dev;
                Out->Function = func;
                Out->Abar = GetAHCIMMIOAddress(bus, dev, func);
                return true;
            }
        }
    }

    return false;
}

static void AHCIResetEnable(hba_mem_t* Hba)
{
    u64 start = GetPerformanceCounter();
    Hba->Ghc |= AHCI_GHC_HR;
    while(true)
    {
        if((Hba->Ghc & AHCI_GHC_HR) == 0)
        {
            break;
        }

        //AHCI仕様に基づき、最大1秒待つ
        if(PerformanceCounterTickToMs(GetPerformanceCounter()-start) > 1000)
        {
            PANIC(STATUS_DISK_IO_ERROR, 1);
        }
    }

    Hba->Ghc |= AHCI_GHC_AE;
}

static void StopPort(hba_port_t* Port)
{
    u64 start = GetPerformanceCounter();
    Port->Cmd &= ~HBA_PxCMD_ST;
    Port->Cmd &= ~HBA_PxCMD_FRE;
    while(true)
    {
        if((Port->Cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR)) == 0)
        {
            break;
        }

        //AHCI仕様に基づき、最大1秒待つ
        if(PerformanceCounterTickToMs(GetPerformanceCounter()-start) > 1000)
        {
            PANIC(STATUS_DISK_IO_ERROR, 1);
        }
    }
}

static void StartPort(hba_port_t* Port)
{
    Port->Cmd |= HBA_PxCMD_FRE;
    Port->Cmd |= HBA_PxCMD_ST;
}

static const char* PortTypeToString(u32 Sig)
{
    switch(Sig)
    {
        case SATA_SIG_SATA:
            return "SATA";
        case SATA_SIG_ATAPI:
            return "SATAPI";
        case SATA_SIG_SEMB:
            return "SEMB";
        case SATA_SIG_PM:
            return "PORT_MULT";
        default:
            return "UNKNOWN/NONE";
    }
}

void AHCIProbeAndListPorts(void)
{
    ahci_pci_info_t info;
    if(!FindAHCIController(&info))
    {
        puts("No AHCI Controller found.\r\n");
    }

    puts("AHCI Device @PCI bus=");
    puts(utos(info.Bus));
    puts(" dev=");
    puts(utos(info.Device));
    puts(" func=");
    puts(utos(info.Function));
    puts("\r\n");

    hba_mem_t* hba = (hba_mem_t*)info.Abar;
    AHCIResetEnable(hba);
    puts("AHCI Version=");
    puts(utos(hba->Vs));
    puts(" CAP=");
    puts(utos(hba->Cap));
    puts("\r\n");

    u32 pi = hba->Pi;
    for(u8 i=0; i<32; i++)
    {
        if((pi & (1u << i)) == 0)
        {
            continue;
        }

        hba_port_t* p = HBA_PORT(hba, i);

        u32 ssts = p->Ssts;
        u8 det = ssts & 0x0F;
        u8 ipm = (ssts >> 8) & 0x0F;

        if(det != 3 || ipm != 1)
        {
            puts("Port ");
            puts(utos(i));
            puts(" : not present or active\r\n");
            continue;
        }

        const char* type = PortTypeToString(p->Sig);
        puts("Port ");
        puts(utos(i));
        puts(": type=");
        puts(type);
        puts("\r\n");
        StopPort(p);
    }
}