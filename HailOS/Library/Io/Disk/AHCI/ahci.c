#include "ahci.h"
#include "basetype.h"
#include "common.h"
#include "util.h"
#include "status.h"
#include "pci.h"
#include "system_console.h"
#include "string.h"
#include "timer.h"
#include "memutil.h"

ALIGN(1024) static u8 gCmdListMemory[1024];
ALIGN(256) static fis_recv_t gFisRecv;
ALIGN(128) static hba_cmd_table_t gCmdTable;
static u8 gIdentifyBuf[512];

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

static int FindFreeSlot(hba_port_t* Port)
{
    u32 slots = Port->Sact | Port->Ci;
    for(int i=0; i<32; i++)
    {
        if((slots & (1u << i)) == 0)
        {
            return i;
        }
    }

    return -1;
}

u32 AHCIInitPort(hba_mem_t* Abar)
{
    u32 pi = Abar->Pi;
    for(int i=0; i<32; i++)
    {
        if(!(pi & (1 << i)))
        {
            continue;
        }

        hba_port_t* port = HBA_PORT(Abar, i);
        ahci_device_type_t type = CheckType(port);
        if(type == AHCI_DEV_SATA)
        {
            StopPort(port);

            port->Clb = 0;
            port->Clbu = 0;
            port->Fb = 0;
            port->Fbu = 0;

            StartPort(port);

            return i;
        }
    }

    return -1;
}

static void AHCIRebasePort(hba_mem_t* Abar, int PortIndex)
{
    hba_port_t* port = HBA_PORT(Abar, PortIndex);

    FillMemory(gCmdListMemory, sizeof(gCmdListMemory), 0);
    FillMemory(&gFisRecv, sizeof(gFisRecv), 0);
    FillMemory(&gCmdTable, sizeof(gCmdTable), 0);

    port->Clb = (u32)(addr_t)gCmdListMemory;
    port->Clbu = 0;
    port->Fb = (u32)(addr_t)&gFisRecv;
    port->Fbu = 0;

    hba_cmd_header_t* hdr = (hba_cmd_header_t*)(addr_t)gCmdListMemory;
    hdr->Cfl = sizeof(fis_reg_h2d_t) / 4;
    hdr->W = 0;
    hdr->Prdtl = 1;
    hdr->Ctba = (u32)(addr_t)&gCmdTable;
    hdr->Ctbau = 0;
    hdr->Prdbc = 0;
    hdr->Pmp = 0;
    hdr->Rsv0 = 0;
    FillMemory(&hdr->Rsv1, sizeof(u32)*4, 0);
    
    hba_cmd_table_t* cmdtbl = &gCmdTable;
    FillMemory(cmdtbl->Cfis, sizeof(cmdtbl->Cfis), 0);
    FillMemory(cmdtbl->Acmd, sizeof(cmdtbl->Acmd), 0);
    FillMemory(cmdtbl->Prdt, sizeof(cmdtbl->Prdt), 0);
}

bool AHCIIdentifyDevice(hba_mem_t* Abar, int PortIndex)
{
    hba_port_t* port = HBA_PORT(Abar, PortIndex);

    u64 start = GetPerformanceCounter();
    while(port->Tfd & (ATA_TFD_BSY | ATA_TFD_DRQ))
    {
        if(PerformanceCounterTickToMs(GetPerformanceCounter()-start) > 1000)
        {
            PANIC(STATUS_DISK_IO_ERROR, 2);
        }
    }

    port->Clb = (u32)(addr_t)gCmdListMemory;
    port->Clbu = 0;
    port->Fb = (u32)(addr_t)&gFisRecv;
    port->Fbu = 0;

    hba_cmd_header_t* hdr = (hba_cmd_header_t*)(addr_t)gCmdListMemory;
    hdr->Cfl = sizeof(fis_reg_h2d_t)/4;
    hdr->W = 0;
    hdr->Prdtl = 1;
    hdr->Ctba = (u32)(addr_t)&gCmdTable;
    hdr->Ctbau = 0;
    hdr->Prdbc = 0;

    hba_prdt_entry_t* prdt = &gCmdTable.Prdt[0];
    prdt->Dba = (u32)(addr_t)gIdentifyBuf;
    prdt->Dbau = 0;
    prdt->DbcI = (512 - 1) | (1u << 31);

    fis_reg_h2d_t* cfis = (fis_reg_h2d_t*)gCmdTable.Cfis;
    for(int i=0; i<64; i++)
    {
        ((u8*)cfis)[i] = 0;
    }

    cfis->FisType = FIS_TYPE_REG_H2D;
    cfis->C = 1;
    cfis->Command = ATA_CMD_IDENTIFY_DEVICE;
    cfis->Device = 1 << 6;

    port->Is = (u32)0xFFFFFFFF;

    port->Ci = 1 << 0;

    start = GetPerformanceCounter();
    while(port->Ci & (1 << 0))
    {
        if(port->Is & TFES_BIT)
        {
            PANIC(STATUS_DISK_IO_ERROR, 3);
        }

        if(PerformanceCounterTickToMs(GetPerformanceCounter() - start) > 1000)
        {
            PANIC(STATUS_DISK_IO_ERROR, 4);
        }
    }

    if(port->Is & TFES_BIT)
    {
        PANIC(STATUS_DISK_IO_ERROR, 5);
    }

    puts("IDENTIFY SUCCESS\r\n");
    return true;
}

void InitSATA(void)
{
    ahci_pci_info_t info;
    if(!FindAHCIController(&info))
    {
        puts("No AHCI Controller found.\r\n");
        return;
    }

    puts("AHCI Device @PCI bus=");
    puts(utos(info.Bus));
    puts(" dev=");
    puts(utos(info.Device));
    puts(" func=");
    puts(utos(info.Function));
    puts("\r\n");

    hba_mem_t* abar = (hba_mem_t*)info.Abar;
    int port = AHCIInitPort(abar);
    AHCIRebasePort(abar, port);
    AHCIIdentifyDevice(abar, port);
}

bool AHCIReadSector(hba_mem_t* Abar, int PortIndex, u64 Lba, u8* Buf)
{
    if(Buf == NULL)
    {
        return false;
    }

    hba_port_t* port = HBA_PORT(Abar, PortIndex);

    hba_cmd_header_t* hdrs = (hba_cmd_header_t*)(addr_t)gCmdListMemory;
    hba_cmd_header_t* cmdheader = &hdrs[0];

    cmdheader->Cfl = sizeof(fis_reg_h2d_t)/sizeof(u32);
    cmdheader->W = 0;
    cmdheader->Prdtl = 1;

    FillMemory(&gCmdTable, sizeof(gCmdTable), 0);

    hba_prdt_entry_t* prdt = &gCmdTable.Prdt[0];
    prdt->Dba = (u32)(addr_t)Buf;
    prdt->Dbau = ((u64)Buf >> 32);
    prdt->DbcI = (512-1) | (1u << 31);

    fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)(&gCmdTable.Cfis);
    FillMemory(cmdfis, sizeof(fis_reg_h2d_t), 0);

    cmdfis->FisType = FIS_TYPE_REG_H2D;
    cmdfis->C = 1;
    cmdfis->Command = ATA_CMD_READ_DMA_EXT;
    cmdfis->Lba0 = (u8)(Lba & 0xFF);
    cmdfis->Lba1 = (u8)((Lba >> 8) & 0xFF);
    cmdfis->Lba2 = (u8)((Lba >> 16) & 0xFF);
    cmdfis->Device = 1 << 6;
    cmdfis->Lba3 = (u8)((Lba >> 24) & 0xFF);
    cmdfis->Lba4 = (u8)((Lba >> 32) & 0xFF);
    cmdfis->Lba5 = (u8)((Lba >> 40) & 0xFF);

    cmdfis->Countl = 1;
    cmdfis->Counth = 0;

    cmdheader->Ctba = (u32)(addr_t)&gCmdTable;
    cmdheader->Ctbau = 0;

    port->Is = 0xFFFFFFFF;

    port->Ci = 1 << 0;

    u64 start = GetPerformanceCounter();
    while(true)
    {
        if((port->Ci & (1 << 0)) == 0)
        {
            break;
        }
        
        if(port->Is & HBA_PxIS_TFES)
        {
            return false;
        }

        if(PerformanceCounterTickToMs(GetPerformanceCounter()-start)>1000)
        {
            return false;
        }
    }

    return true;
}