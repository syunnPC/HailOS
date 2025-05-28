#pragma once

#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
#endif

#ifndef PACKED_STRUCTURE
#define PACKED_STRUCTURE __attribute__((packed))
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#define SI_KI (1024)
#define SI_MI (1024*1024)
#define SI_GI (1024*1024*1024)
#define SI_KILO (1000)
#define SI_MEGA (1000*1000)
#define SI_GIGA (1000*1000*1000)

extern void Panic(unsigned int, unsigned int, unsigned int);

#define PANIC(Code1, Code2) Panic(Code1, Code2, __LINE__)