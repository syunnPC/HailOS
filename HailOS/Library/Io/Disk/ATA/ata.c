#include "ata.h"
#include "io.h"
#include "basetype.h"
#include "common.h"
#include "util.h"
#include "status.h"

void AtaReadSectorLba28(u32 Lba, u8* Buffer)
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