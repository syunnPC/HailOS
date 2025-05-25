#pragma once

#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
#endif

#ifndef PACKED_STRUCTURE // Consistent naming with your usage
#define PACKED_STRUCTURE __attribute__((packed))
#endif

#ifndef NULL
#define NULL ((void*)0) // Cast to void*
#endif

#define SI_KI 1024
#define SI_MI 1024*SI_KI
#define SI_GI 1024*SI_MI

extern void Panic(unsigned int, unsigned int, unsigned int);

#define PANIC(Code1, Code2) Panic(Code1, Code2, __LINE__)