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

#if !defined __GNUC__
#error Use x86_64-elf-gcc to compile this program.
#endif

void main(bootinfo_t* Info)
{
    InitSystem(Info);

    puts("HailOS Version 0.0.2r\r\n");
    puts(utos(GetAvailableMemorySize() / SI_MEGA));
    puts("MB Memory available. Largest block size = ");
    puts(utos(GetLargestMemoryRegion() / SI_MEGA));
    puts("\r\n"); 
    puts("Screen resolution : ");
    puts(utos(GetScreenResolution().Width));
    puts(" x ");
    puts(utos(GetScreenResolution().Height));
    puts("\r\n");

    while(true)
    {
        //puts()のところが2回表示されたり不正な分岐で実行される
        char filename[13];
        puts(">");
        ReadInputWithEcho(filename, 13, COLOR_WHITE, true);
        if(!IsExistingFile(filename))
        {
            puts("The file does not exist.\r\n");
            continue;
        }
        DrawBitmapInline(filename);
    }
}