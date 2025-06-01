#include "fat32.h"
#include "io.h"
#include "ata.h"
#include "basetype.h"
#include "util.h"
#include "memutil.h"
#include "status.h"
#include "string.h"
#include "system_console.h"
#include "timer.h"

static fat32_vbr_t sVbr;
static u64 sFat32BaseLba;
static u32 sSectorsPerCluster;
static u32 sFatStartLba;
static u32 sClusterStartLba;
static u32 sRootCluster;
static u8 sFatSector[SECTOR_SIZE];

static const u8 ESP_GUID[16] = {
    0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
    0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
};

static const u8 MS_BASIC_DATA_GUID[16] = {
    0xA2, 0xA0, 0xD0, 0xEB, 0xE5, 0xB9, 0x33, 0x44,
    0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7
};

bool IsGptProtectiveMbr(u8* Sector)
{
    mbr_partition_entry_t* entry = (mbr_partition_entry_t*)(Sector+446);
    return (entry->Type == 0xEE);
}

void ReadCluster(u32 ClusterNum, u8* Buffer)
{
    u32 first_lba = sClusterStartLba + (ClusterNum - 2) * sSectorsPerCluster;
    for(u32 i=0; i<sSectorsPerCluster; i++)
    {
        AtaReadSectorLba28((u32)first_lba+i, Buffer + (i*SECTOR_SIZE));
    }
}

u32 GetNextCluster(u32 Cluster)
{
    u32 fat_offset = Cluster * 4;
    u32 fat_sector_num = fat_offset / SECTOR_SIZE;
    u32 offset_in_sector = fat_offset % SECTOR_SIZE;

    u32 fat_read_lba = sFatStartLba + fat_sector_num;

    AtaReadSectorLba28(fat_read_lba, sFatSector);

    u32 raw_next_cluster_val;
    if (offset_in_sector <= SECTOR_SIZE - sizeof(u32))
    {
        u8* p_fat_entry = sFatSector + offset_in_sector;
        raw_next_cluster_val = (u32)p_fat_entry[0] | ((u32)p_fat_entry[1] << 8) | ((u32)p_fat_entry[2] << 16) | ((u32)p_fat_entry[3] << 24);
    } 
    else 
    {
        PANIC(STATUS_IO_ERROR, offset_in_sector);
    }

    u32 final_next_cluster = raw_next_cluster_val & 0x0FFFFFFF;
    return final_next_cluster;
}

bool CompareFileName(const u8* FatName, const char* Target)
{
    char formatted[12] = {0};
    size_t len = strlen(Target);
    size_t dot = len;
    for(size_t i=0; i<len; i++)
    {
        if(Target[i] == '.')
        {
            dot = i;
            break;
        }
    }
    FillMemory(formatted, ' ', 11);
    MemCopy(formatted, Target, dot > 8 ? 8 : dot);
    if(dot < len - 1)
    {
        MemCopy(formatted + 8, &Target[dot + 1], (len - dot - 1 > 3) ? 3 : len-dot-1);
    }
    for(int i=0; i<11; i++)
    {
        if((FatName[i] & 0xDF) != (formatted[i] & 0xDF))
        {
            return false;
        }
    }
    return true;
}

HOSstatus ReadFile(const char* FileName, u8* OutBuffer, size_t MaxSize, size_t* OutSize)
{
    sSectorsPerCluster = sVbr.SectorsPerCluster;
    sFatStartLba = sFat32BaseLba + sVbr.ReservedSectorCount;
    sClusterStartLba = sFat32BaseLba + sVbr.ReservedSectorCount + (sVbr.NumFats * sVbr.FatSize32);
    sRootCluster = sVbr.RootClustor;

    u32 current_cluster = sRootCluster;
    u32 cluster_buffer_size = sSectorsPerCluster * SECTOR_SIZE;
    u8* cluster_buf = AllocInitializedMemory(cluster_buffer_size, 0);

    if(cluster_buf == NULL)
    {
        return STATUS_MEMORY_ALLOCATION_FAILED;
    }

    while(current_cluster < 0x0FFFFFF8)
    {
        ReadCluster(current_cluster, cluster_buf);

        for(u32 i=0; i < cluster_buffer_size; i+=32)
        {
            u8* entry = &cluster_buf[i];
            
            if(entry[0] == 0x00)        
            {
                FreeMemory(cluster_buf, cluster_buffer_size);
                return STATUS_ERROR;
            }
            //コンパイル時最適化に注意→volatile指定が必要かも
            if(entry[0] == 0xE5) { continue; }
            if((entry[11] & 0x3F) == 0x0F) {continue;}
            if(entry[11] & 0x08) { continue; }
            if(CompareFileName(entry, FileName))
            {
                u32 file_cluster =  (u32)entry[26] | ((u32)entry[27] << 8) | ((u32)entry[20] << 16) | ((u32)entry[21] << 24);
                u32 filesize = *(u32*)&entry[28];

                if(OutSize != NULL)
                {
                    *OutSize = filesize;
                }

                u32 bytes_read = 0;
                while(file_cluster < 0x0FFFFFF8 && bytes_read < MaxSize)
                {
                    ReadCluster(file_cluster, cluster_buf);
                    u32 amount_remaining_in_file = filesize - bytes_read;
                    u32 amount_can_copy_from_cluster = cluster_buffer_size;
                    u32 copy_size = (amount_remaining_in_file < amount_can_copy_from_cluster) ? amount_remaining_in_file : amount_can_copy_from_cluster;
                    
                    if (bytes_read + copy_size > MaxSize) 
                    {
                        copy_size = MaxSize - bytes_read;
                    }

                    if (copy_size == 0 && filesize > bytes_read) 
                    {
                         break; 
                    }

                    MemCopy((void*)OutBuffer+bytes_read, cluster_buf, copy_size);
                    bytes_read += copy_size;
                    
                    if (bytes_read >= filesize) 
                    {
                        break;
                    }

                    file_cluster = GetNextCluster(file_cluster);
                }
                FreeMemory(cluster_buf, cluster_buffer_size);
                return STATUS_SUCCESS;
            }
        }
        current_cluster = GetNextCluster(current_cluster);
    }

    FreeMemory(cluster_buf, cluster_buffer_size);
    return STATUS_ERROR;
}

bool FindFat32Partition(u64* OutLba)
{
    gpt_header_t header;
    AtaReadSectorLba28(1, (u8*)&header);
    if(header.Signature != GPT_SIGNATURE)
    {
        return false;
    }

    u64 entry_lba = header.PartitionEntryLba;
    u32 entry_count = header.NumPartitionEntries;
    u32 entry_size = header.SizeOfPartitionEntry;
    gpt_entry_t entry;
    u8 sector_buffer[SECTOR_SIZE];
    fat32_vbr_t vbr_temp;

    for(u32 i=0; i<entry_count; i++)
    {
        if(entry_size == 0)
        {
            PANIC(STATUS_DISK_IO_ERROR, 21);
        }
        u32 target_lba = entry_lba + (i * entry_size) / SECTOR_SIZE;
        u32 offset_in_sector = (i * entry_size) % SECTOR_SIZE;
        AtaReadSectorLba28(target_lba, sector_buffer);
        MemCopy(&entry, sector_buffer + offset_in_sector, entry_size);

        if(entry.FirstLba > 0)
        {
            bool guid_match = false;
            if(MemEq(entry.PartitionTypeGuid, ESP_GUID, 16))
            {
                //puts("Found EFI System Partition.\r\n");
                guid_match = true;
            }
            else if(MemEq(entry.PartitionTypeGuid, MS_BASIC_DATA_GUID, 16))
            {
                //puts("Found Microsoft Basic DATA Partition.\r\n");
                guid_match = true;
            }

            if(guid_match)
            {
                AtaReadSectorLba28(entry.FirstLba, (u8*)&vbr_temp);
                if(strncmp(vbr_temp.FsType, "FAT32   ", 5) == 0)
                {
                    *OutLba = entry.FirstLba;
                    return true;
                }
            }
        }
    }
    return false;
}

void InitVbr()
{
    if(!FindFat32Partition(&sFat32BaseLba))
    {
        PANIC(STATUS_NOT_FOUND, 0);
    }
    AtaReadSectorLba28(sFat32BaseLba, (u8*)&sVbr);

    if (sVbr.BytesPerSector != 512 || sVbr.SectorsPerCluster == 0 || (sVbr.SectorsPerCluster & (sVbr.SectorsPerCluster - 1)) != 0 || sVbr.NumFats == 0 || 
    sVbr.ReservedSectorCount == 0 ||  sVbr.FatSize32 == 0 || sVbr.RootClustor < 2 || sVbr.BootSectorSignature != 0xAA55 || strncmp(sVbr.FsType, "FAT32", 5) != 0)
    {
        PANIC(STATUS_ASSERTION_FAILED, 1);
    }

    sSectorsPerCluster = sVbr.SectorsPerCluster;
    sFatStartLba = sFat32BaseLba + sVbr.ReservedSectorCount;
    sClusterStartLba = sFat32BaseLba + sVbr.ReservedSectorCount + (sVbr.NumFats * sVbr.FatSize32);
    sRootCluster = sVbr.RootClustor;
}