#define ATA_DATA_PORT       0x1F0
#define ATA_ERROR_PORT      0x1F1
#define ATA_SECTOR_COUNT    0x1F2
#define ATA_LBA_LOW         0x1F3
#define ATA_LBA_MID         0x1F4
#define ATA_LBA_HIGH        0x1F5
#define ATA_DRIVE_SELECT    0x1F6
#define ATA_COMMAND_PORT    0x1F7
#define ATA_STATUS_PORT     0x1F7
#define AtA_CTRL_BASE       0x3F6

#define ATA_CMD_READ_SECTORS 0x20
#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08

#include "typelib.h"

void AtaReadSectorLba28(u32, u8*);
void AtaReadSectorLba48(u64, u8*);