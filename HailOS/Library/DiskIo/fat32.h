#include "typelib.h"
#include "commonlib.h"

typedef struct 
{
    u8 Status;
    u8 ChsFirst[3];
    u8 Type;
    u8 ChsLast[3];
    u32 LbaStart;
    u32 TotalSectors;
} PACKED_STRUCTURE mbr_partition_entry_t;

typedef struct
{
    u8 PartitionTypeGuid[16];
    u8 UniquePartitionGuid[16];
    u64 FirstLba;
    u64 LastLba;
    u64 Attributes;
    char16 PartitionName[36];
} PACKED_STRUCTURE gpt_entry_t;

typedef struct
{
    char Signature[8];
    u32 Revsion;
    u32 HeaderSize;
    u32 HeaderCrc32;
    u32 Reserved;
    u64 CurrentLba;
    u64 BackupLba;
    u64 FirstAvailableLba;
    u64 LastAvailableLba;
    u8 DiskGuid[16];
    u64 PartitionEntryLba;
    u32 NumPartitionEntries;
    u32 SizeOfPartitionEntry;
    u32 PartitionEntriesCrc32;

    u8 Padding[420];
} PACKED_STRUCTURE gpt_header_t;

typedef struct
{
    u8 JmpInstr[3];
    char OemName[8];
    u16 BytesPerSector;
    u8 SectorsPerCluster;
    u16 ReservedSectorCount;
    u8 NumFats;
    u16 RootEntryCount;
    u16 TotalSectors16;
    u8 MediaType;
    u16 FatSize16;
    u16 SectorsPerTrack;
    u16 NumHeads;
    u32 HiddenSectors;
    u32 TotalSectors32;
    u32 FatSize32;
    u16 ExtFlags;
    u16 FsVersion;
    u32 RootClustor;
    u16 FsInfo;
    u16 BackupBootSector;
    u8 Reserved[12];
    u8 DriveNumber;
    u8 Reserved1;
    u8 BootSignature;
    u32 VolumeId;
    char VolumeLabel[11];
    char FsType[8];
    u8 BootCode[420];
    u16 BootSectorSignature; //0xAA55
} PACKED_STRUCTURE fat32_vbr_t;

#define SECTOR_SIZE 512
#define MAX_PATH 128

boolean IsGPTProtectiveMbr(u8*);

extern fat32_vbr_t Vbr;
extern u64 Fat32BaseLba;
extern u32 SectorsPerCluster;
extern u32 FatStartLba;
extern u32 ClusterStartLba;
extern u32 RootCluster;
extern u8 FatSector[SECTOR_SIZE];

void ReadCluster(u32, void*);
u32 GetNextCluster(u32);
boolean CompareFileName(const u8*, const char*);

boolean ReadFile(const char*, void*, size_t, size_t*);
boolean FindFat32Partition(u64*);
void InitVbr();

#define GPT_SIGNATURE 0x5452415020494645ULL