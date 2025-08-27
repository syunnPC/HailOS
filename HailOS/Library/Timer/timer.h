/*
    時間測定ライブラリ
    作成日 2025-05-30
*/

#pragma once

#include "timedef.h"
#include "basetype.h"

/// @brief 内部タイマーを初期化
/// @param  ClockInfo ブートローダーから渡されるhw_clockinfo_t構造体
void InitTime(hw_clockinfo_t*);

/// @brief 現在のUNIXタイムを取得
/// @return UNIXタイム
time_t GetCurrentUnixTime(void);

/// @brief パフォーマンスカウンタを取得
/// @return 呼び出し時のTSCの値
u64 GetPerformanceCounter(void);

/// @brief パフォーマンスカウンタの差の値からミリ秒へ変換
/// @param  Delta パフォーマンスカウンタの差
/// @return 変換された時間（ミリ秒）
u64 PerformanceCounterTickToMs(u64);

/// @brief システムの起動時間を取得
/// @return システムが起動してからの時間
time_t GetSystemUpTime(void);

/// @brief 指定されたミリ秒スリープ
/// @param  Ms スリープする秒数（ミリ秒）
void Sleep(u64);

/// @brief TSC周波数を取得
/// @return TSC周波数
u64 GetTscFreq(void);