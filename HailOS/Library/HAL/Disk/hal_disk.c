#include "hal_disk.h"
#include "basetype.h"
#include "ata.h"
#include "ahci.h"
#include "util.h"
#include "status.h"

static disk_t sInternalDiskInfo;

disk_type_t HALInitDisk(void)
{
    ahci_pci_info_t pi;
    sInternalDiskInfo.Abar = NULL;
    sInternalDiskInfo.PortIndex = 0;
    if(FindAHCIController(&pi))
    {
        sInternalDiskInfo.Type = DISK_TYPE_AHCI;
        sInternalDiskInfo.Abar = (hba_mem_t*)pi.Abar;
        InitSATA();
        return DISK_TYPE_AHCI;
    }
    else if(ATACheckDeviceMaster())
    {
        sInternalDiskInfo.Type = DISK_TYPE_IDE;
        return DISK_TYPE_IDE;
    }
    else
    {
        return DISK_TYPE_NONE;
    }

    return DISK_TYPE_NONE;
}

bool HALDiskReadSector(disk_t* Disk, u64 Lba, u8* Buf)
{
    if(Buf == NULL)
    {
        return false;
    }

    switch(Disk->Type)
    {
        case DISK_TYPE_IDE:
            ATAReadSectorLBA28((u32)Lba, Buf);
            return true;
        case DISK_TYPE_AHCI:
            return AHCIReadSector(Disk->Abar, Disk->PortIndex, Lba, Buf);
        default:
            PANIC(STATUS_DISK_IO_ERROR, (u64)Disk->Type);
    }
}

bool HALMainDiskReadSector(u64 Lba, u8* Buf)
{
    return HALDiskReadSector(&sInternalDiskInfo, Lba, Buf);
}