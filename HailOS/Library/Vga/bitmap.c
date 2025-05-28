#include "bitmap.h"
#include "typelib.h"
#include "vga.h"
#include "print.h"
#include "status.h"
#include "fileio.h"
#include "stdcolor.h"

extern graphic_info_t* gGraphicInfo;

HOSstatus DrawBitmap(const char* FileName, coordinate2D_t Location)
{
    HOSstatus Status;
    file_object_t File;
    Status = OpenFile(FileName, &File);
    if(HOS_ERROR(Status))
    {
        return Status;
    }

    bitmapfileheader_t bf = *(bitmapfileheader_t*)File.Buffer;
    bitmapinfoheader_t bi = *(bitmapinfoheader_t*)((size_t)File.Buffer+sizeof(bitmapfileheader_t));
    if(bf.bfType[0] != 'B' || bf.bfType[1] != 'M')
    {
        CloseFile(&File);
        return STATUS_UNSUPPORTED;
    }

    if(bi.biCompression != BI_RGB)
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    if(bi.bcBitCount != 32)
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    if(bi.bcHeight <= 0)
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    if(bi.bcWidth % 4 != 0)
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    i32 Height = bi.bcHeight;
    u32 Width = bi.bcWidth;
    size_t drawPx = 0;

    for(i32 y=Height; y>=0; y--)
    {
        for(u32 x = 0; x<Width; x++)
        {
            rgbquad_t q = *((rgbquad_t*)((size_t)File.Buffer + bf.bfOffBits)+drawPx++);
            DrawPixelToBuffer(COORD(Location.X+x, Location.Y+y), RGB_COLOR(q.rgbRed, q.rgbGreen, q.rgbBlue));
        }
    }

    DrawBufferContextToFrameBuffer();
    CloseFile(&File);
    return STATUS_SUCCESS;
}