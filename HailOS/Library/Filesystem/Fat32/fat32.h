/*
    FAT32 読み取り用定義
    作成日 2025-05-30
*/

#pragma once

#include "basetype.h"
#include "common.h"

#define SECTOR_SIZE 512
#define MAX_PATH 128
#define GPT_SIGNATURE 0x5452415020494645ULL

typedef struct 
{
    u8 Status;
    u8 ChsFirst[3];
    u8 Type;
    u8 ChsLast[3];
    u32 LbaStart;
    u32 TotalSectors;
} PACKED mbr_partition_entry_t;

typedef struct
{
    u8 PartitionTypeGuid[16];
    u8 UniquePartitionGuid[16];
    u64 FirstLba;
    u64 LastLba;
    u64 Attributes;
    char16 PartitionName[36];
} PACKED gpt_entry_t;

typedef struct
{
    u64 Signature;
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
} PACKED gpt_header_t;

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
} PACKED fat32_vbr_t;

/// @brief GPT保護MBRかをチェックする
/// @param  Sector 調べるセクター
/// @return GPT保護MBRならtrue, そうでないならfalse
bool IsGptProtectiveMbr(u8*);

/// @brief クラスタを読みだす
/// @param u32 ClusterNum クラスタ番号
/// @param  Buffer 書き出すバッファ 
void ReadCluster(u32, u8*);

/// @brief 次のクラスタ番号を取得する
/// @param  Cluster 求めるクラスタ
/// @return 求めるクラスタの次のクラスタ番号
u32 GetNextCluster(u32);

/// @brief ファイル名が等しいか比較
/// @param  FatName FAT名
/// @param  Target 比較する名前
/// @return 等しいならtrue, そうでないならfalse
bool CompareFileName(const u8*, const char*);

/// @brief ファイルを読み込む
/// @param  FileName ファイル名
/// @param  OutBuffer 書き出すバッファ
/// @param  MaxSize 読みだす最大サイズ
/// @param  OutSize 実際に読み込んだファイルサイズ
/// @return 正常に読み込めば STATUS_SUCCESS, 読み込めなかった場合はSTATUS_ERROR
HOSstatus ReadFile(const char*, u8*, size_t, size_t*);

/// @brief FAT32パーティションを見つける
/// @param  OutLba 最初のLBA番号
/// @return 見つかったならtrue, なければfalse
bool FindFat32Partition(u64*);

/// @brief VBRを初期化
void InitVbr(void);