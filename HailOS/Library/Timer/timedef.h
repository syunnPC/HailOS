/*
    時間測定に必要な構造体
    作成日 2025-05-30
*/

#pragma once

#include "common.h"
#include "basetype.h"

typedef struct
{
    time_t InitialUnixTime;
    u64 InitialTsc;
    u64 TscFreq;
}PACKED hw_clockinfo_t;