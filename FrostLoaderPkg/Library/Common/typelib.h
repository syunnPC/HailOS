#pragma once

typedef unsigned char u8, byte, uchar, boolean, char8;
typedef unsigned short u16, word, ushort, char16;
typedef unsigned int u32, dword, uint, char32;
typedef unsigned long long u64, qword, ulong, size_t;

typedef signed char i8, schar;
typedef short i16;
typedef int i32;
typedef long long i64, ssize_t;

typedef u64 addr_t;

typedef u32 HOSstatus;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif