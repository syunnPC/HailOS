#pragma once

#include "typelib.h"

size_t strlen(const char*);
void* memcpy(void*, const void*, size_t);
boolean memeq(const void*, const void*, size_t);
boolean strcmp(const char*, const char*);
void* memset(void*, char, size_t);
boolean strncmp(const char*, const char*, size_t);
char* strstr(char*, const char*);
char* nstd_strcat_s(const char*, const char*);
char* itos(i64);
char* utos(u64);