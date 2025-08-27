#include "ata.h"
#include "io.h"
#include "basetype.h"
#include "common.h"
#include "util.h"
#include "status.h"
#include "timer.h"

#define SLEEP_LENGTH 500

static inline void IoWait(void)
{
    u64 f = GetTscFreq();
    double cpns = f / 1000000000;
    u64 wclk = (u64)(cpns * SLEEP_LENGTH);
    u64 start = GetPerformanceCounter();
    while(!(GetPerformanceCounter() > start + wclk));
}

bool ATACheckDeviceMaster(void)
{
    outb(ATA_DATA_PORT + ATA_REG_DRIVE_HEAD, 0xA0);
    IoWait();

    outb(ATA_DATA_PORT + ATA_REG_STATUS, 0x00);
    IoWait();

    outb(ATA_DATA_PORT+ ATA_REG_STATUS, ATA_CMD_IDENTIFY);
    IoWait();

    u8 status = inb(ATA_DATA_PORT+ATA_REG_STATUS);
    if(status == 0)
    {
        return false;
    }

    u64 f = GetTscFreq();
    double cpns = f / 1000000000;
    u64 wclk = (u64)(cpns * SLEEP_LENGTH);
    u64 start = GetPerformanceCounter();
    while(inb(ATA_DATA_PORT + ATA_REG_STATUS) & ATA_STATUS_BSY)
    {
        if(GetPerformanceCounter() - start > wclk)
        {
            return false;
        }
    }

    if(PerformanceCounterTickToMs(GetPerformanceCounter()-start) >= 1000)
    {
        return false;
    }

    if(status & ATA_STATUS_ERR)
    {
        return false;
    }

    if(status & ATA_STATUS_DRQ)
    {
        return true;
    }

    return false;
}

void ATAReadSectorLBA28(u32 Lba, u8* Buffer)
{
    if(Buffer == NULL)
    {
        return;
    }
    while(inb(ATA_STATUS_PORT) & ATA_STATUS_BSY);

    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (u8)(Lba & 0xFF));
    outb(ATA_LBA_MID, (u8)((Lba>>8) & 0xFF));
    outb(ATA_LBA_HIGH, (u8)((Lba>>16) & 0xFF));
    outb(ATA_DRIVE_SELECT, 0xE0 | ((Lba>>24) & 0x0F));
    outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);

    while(true)
    {
        u8 status = inb(ATA_STATUS_PORT);
        if(status & ATA_STATUS_ERR)
        {   
            PANIC(STATUS_DISK_IO_ERROR, (u64)status);
        }
        if(!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ))
        {
            break;
        }
    }

    for(int i=0; i<256; i++)
    {
        u16 word = inw(ATA_DATA_PORT);
        Buffer[i*2] = (u8)(word & 0xFF);
        Buffer[i*2 + 1] = (u8)((word >> 8) & 0xFF);
    }
}

