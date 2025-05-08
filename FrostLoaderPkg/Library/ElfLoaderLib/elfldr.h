#pragma once

#include <Uefi.h>
#include "Common/commonlib.h"
#include "Common/typelib.h"

EFI_STATUS LoadFileToMemory(CHAR16*, VOID**, UINTN*);
EFI_STATUS LoadElfExecutable(VOID*, UINTN, UINT64*);
EFI_STATUS PrepareElfExecutable(CHAR16*, UINT64*);