/*
    共通定義（マクロ）
    作成日 2025-05-29
*/

#pragma once

#ifndef NULL
#define NULL ((void*)0);
#endif //NULL

#define NORETURN __attribute__((noreturn))
#define PACKED __attribute__((packed))

#define SI_KILO 1000
#define SI_MEGA 1000000
#define SI_GIGA 1000000000