#pragma once

#include "typelib.h"

size_t strlen(const char*);
void* memcpy(void*, const void*, size_t);
boolean memcmp(void*, void*, size_t);
boolean strcmp(const char*, const char*);
void* memset(void*, char, size_t);
boolean strncmp(const char*, const char*, size_t);
void* strstr(const char*, const char*);