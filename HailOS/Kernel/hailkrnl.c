#include "krnl_init.h"
#include "boot.h"
#include "color.h"
#include "util.h"
#include "vga.h"
#include "system_console.h"
#include "memutil.h"
#include "timer.h"
#include "bitmap.h"
#include "status.h"
#include "string.h"
#include "file.h"
#include "pci.h"
#include "ahci.h"
#include "acpi.h"

void main(bootinfo_t* Info)
{
    InitSystem(Info);
    DrawBitmapInline("picture1.bmp");

    Sleep(10000);
    ACPIShutdown();
}