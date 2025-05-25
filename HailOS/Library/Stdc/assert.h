#pragma once

#include "typelib.h"

#define ASSERT(expr) (expr == false? internal_impl_assert(#expr, __LINE__))

void internal_impl_assert(const char*, u32);