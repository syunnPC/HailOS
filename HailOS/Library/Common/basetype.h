/*
    基本的な型の定義
    作成日 2025-05-29
*/

#pragma once

typedef unsigned char u8, uchar, char8;
typedef unsigned short u16, word, ushort, char16;
typedef unsigned int u32, dword, uint, char32;
typedef unsigned long long u64, qword, ulong, size_t, addr_t, time_t;
typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64, ssize_t;

typedef u32 HOSstatus;

#ifndef bool
#define bool _Bool
#endif

#ifndef true
#define true 1
#endif //false

#ifndef false
#define false 0
#endif //false