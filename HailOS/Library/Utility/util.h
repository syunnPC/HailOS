/*
    OS ユーティリティ機能
    作成日 2025-05-29
*/

#pragma once

#include "basetype.h"
#include "common.h"

/// @brief パニックを発生させ、システムを停止
/// @param  Status ステータスコード
/// @param  Param 問題の特定に役立つパラメータ
/// @param  Line 問題が発生したソースファイルの行数
/// @param FileName ファイル名
NORETURN void Panic(HOSstatus, u64, u32, const char*);

/// @brief 強制再起動を実施（トリプルフォールトを発生させる）
NORETURN void ForceReboot(void);

/// @brief プロセッサを停止
NORETURN void HaltProcessor(void);

//Panic()をコールするマクロ
#define PANIC(Status, Param) Panic(Status, Param, __LINE__, __FILE__)