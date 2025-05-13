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