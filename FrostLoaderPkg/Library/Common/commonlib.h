#if defined __GNUC__ && !defined __clang__

#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
#endif

#ifndef PACKED_STRUCTURE
#define PACKED_STRUCTURE __attribute__((packed))
#endif

#elif

#ifndef NORETURN
#define NORETURN
#endif

#ifndef PACKED
#define PACKED
#endif

#endif