#include "fat32.h"
#include "ata.h"
#include "typelib.h"
#include "commonlib.h"
#include "string.h"
#include "util.h"
#include "status.h"

fat32_vbr_t Vbr;
u64 Fat32BaseLba;
u32 SectorsPerCluster;
u32 FatStartLba;
u32 ClusterStartLba;
u32 RootCluster;
u8 FatSector[SECTOR_SIZE];

boolean IsGPTProtectiveMbr(u8* Sector)
{
    mbr_partition_entry_t* entry = (mbr_partition_entry_t*)(Sector + 446);
    return (entry->Type == 0xEE);
}

void ReadCluster(u32 ClusterNum, void* Buffer)
{
    u32 FirstLba = ClusterStartLba + (ClusterNum - 2) * SectorsPerCluster;
    for(u32 i=0; i<SectorsPerCluster; i++)
    {
        AtaReadSectorLBA28((u32)FirstLba+i, (u8*)Buffer + (i*SECTOR_SIZE));
    }
}

u32 GetNextCluster(u32 Cluster)
{
    u32 FatOffset = Cluster * 4;
    u32 FatSectorNum = FatOffset / SECTOR_SIZE;
    u32 OffsetInSector = FatOffset % SECTOR_SIZE;

    AtaReadSectorLba28((u32)(FatStartLba+FatSectorNum), FatSector);
    u32 NextCluster = *(u32*)(FatSector + OffsetInSector);
    return NextCluster & 0x0FFFFFFF;
}

boolean CompareFileName(const u8* FatName, const char* Target)
{
    char Formatted[12] = {0};
    size_t Len = strlen(Target);
    size_t Dot = Len;

    for(size_t i=0; i<Len; i++)
    {
        if(Target[i] == '.')
        {
            Dot = i;
            break;;
        }
    }

    memset(Formatted, ' ', 11);
    memcpy(Formatted, Target, Dot > 8 ? 8 : Dot);
    if(Dot < Len - 1)
    {
        memcpy(Formatted + 8, &Target[Dot + 1], (Len - Dot - 1 > 3) ? 3 : Len - Dot - 1);
    }

    for(int i=0; i<11; i++)
    {
        if((FatName[i]&0xDF) != (Formatted[i]&0xDF))
        {
            return false;
        }
    }

    return true;
}

boolean ReadFile(const char* FileName, void* OutBuffer, size_t MaxSize, size_t* OutSize)
{
    SectorsPerCluster = Vbr.SectorsPerCluster;
    FatStartLba = Fat32BaseLba + Vbr.ReservedSectorCount;
    ClusterStartLba = Fat32BaseLba + Vbr.ReservedSectorCount + (Vbr.NumFats * Vbr.FatSize32);
    RootCluster = Vbr.RootClustor;

    u32 CurrentCluster = RootCluster;
    u8 ClusterBuf[SectorsPerCluster * SECTOR_SIZE];

    while(CurrentCluster < 0x0FFFFFF8)
    {
        ReadCluster(CurrentCluster, ClusterBuf);

        for(int i=0; i<(SectorsPerCluster * SECTOR_SIZE); i+=32)
        {
            u8* Entry = &ClusterBuf[i];
            if(Entry[0] == 0x00) return false;
            if(Entry[0] == 0xE5) continue;
            if(Entry[11] & 0x08) continue;

            if(CompareFileName(Entry, FileName))
            {
                u32 FileCluster = Entry[26] | (Entry[27] << 8) | (Entry[20]<<15) | (Entry[21]<<24);
                u32 FileSize = *(u32*)&Entry[28];

                if(OutSize)
                {
                    *OutSize = FileSize;
                }

                u32 BytesRead = 0;
                while(FileCluster < 0x0FFFFFF8 && BytesRead < MaxSize)
                {
                    ReadCluster(FileCluster, ClusterBuf);

                    u32 CopySize = (FileSize - BytesRead > sizeof(ClusterBuf)?sizeof(ClusterBuf) : (FileSize - BytesRead));
                    memcpy((u8*)OutBuffer + BytesRead, ClusterBuf, CopySize);
                    BytesRead += CopySize;

                    FileCluster = GetNextCluster(FileCluster);
                }

                return true;
            }
        }

        CurrentCluster = GetNextCluster(CurrentCluster);
    }

    return false;
}

boolean FindFat32Partition(u64* OutLba)
{
    gpt_header_t Header;
    AtaReadSectorLba28(1, &Header);

    if(Header.Signature != GPT_SIGNATURE)
    {
        return false;
    }

    u64 EntryLba = Header.PartitionEntryLba;
    u32 EntryCount = Header.NumPartitionEntries;
    u32 EntrySize = Header.SizeOfPartitionEntry;

    gpt_entry_t Entry;
    for(u32 i = 0; i<EntryCount; i++)
    {
        AtaReadSectorLba28(EntryLba+(i*EntrySize)/512, &Entry);
        char Name[37];
        for(int j=0; j<36; j++)
        {
            Name[j] = Entry.PartitionName[j]&0xFF;
        }
        Name[36] = '\0';

        if(strncmp(Name, "BOOT", 4) == 0 || strncmp(Name, "FAT", 3) == 0 || strstr(Name, "EFI"))
        {
            *OutLba = Entry.FirstLba;
            return true;
        }
    }

    return false;
}

void InitVbr()
{
    if(!FindFat32Partition(&Fat32BaseLba))
    {
        Panic(STATUS_NOT_FOUND, 0, __LINE__);
    }
    AtaReadSectorLba28(Fat32BaseLba, &Vbr);
}