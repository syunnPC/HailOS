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

#define COMMAND_LEN_MAX 256

#define COMMAND_NAME_CLEAR "clear"
#define COMMAND_NAME_PIC "pic"
#define COMMAND_NAME_EXIT "exit"
#define COMMAND_NAME_VER "ver"

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

    char cmd[COMMAND_LEN_MAX];
    while(true)
    {
        //puts()のところが2回表示されたり不正な分岐で実行される
        FillMemory(cmd, COMMAND_LEN_MAX, 0);
        puts(">");
        ReadInputWithEcho(cmd, COMMAND_LEN_MAX, COLOR_WHITE, true);
        if(strcmp(COMMAND_NAME_CLEAR, cmd) == 0)
        {
            ClearBuffer();
            FillScreenWithBackgroundColor();
            SetCursorPos(COORD(0, 0));
            continue;
        }
        else if(strcmp(COMMAND_NAME_VER, cmd) == 0)
        {
            puts("HailOS Version 0.0.2\r\nSee https://github.com/syunnPC/HailOS\r\n");
            continue;
        }
        else if(strncmp(COMMAND_NAME_PIC, cmd, strlen(COMMAND_NAME_PIC)) == 0)
        {
            if(cmd[3] == ' ')
            {
                char filename[13];
                filename[12] = '\0';
                MemCopy(filename, (u8*)cmd + 4, FILENAME_MAX);
                if(!IsExistingFile(filename))
                {
                    puts("file not found.\r\n");
                    continue;
                }
                DrawBitmapInline(filename);
                continue;
            }
            else
            {
                continue;
            }
        }

        puts("Command not found.\r\n");
    }
}