/*
    ファイルIO管理
    作成日 2025-06-01
*/

#pragma once 

#include "basetype.h"

#define FILENAME_MAX (12)
#define FILESIZE_MAX (4000000000)

/// @brief ファイルオブジェクトを管理する構造体
typedef struct
{
    u8* Buffer;
    size_t FileSize;
    size_t AllocatedSize;
}file_object_t;

/// @brief ファイルをオープン
/// @param  FileName ファイル名（8.3形式, 12文字まで）
/// @param  Result オープンしたオブジェクトのバッファ
/// @return ステータスコード
HOSstatus OpenFile(const char*, file_object_t*);

/// @brief ファイルをクローズ   
/// @param  Object クローズするファイルのオブジェクト
/// @return ステータスコード
HOSstatus CloseFile(file_object_t*);

/// @brief ファイルが存在するかをチェック
/// @param  FileName ファイル名
/// @return ファイルの存在
bool IsExistingFile(const char*);