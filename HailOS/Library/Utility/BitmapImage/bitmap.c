#include "bitmap.h"
#include "basetype.h"
#include "vga.h"
#include "status.h"
#include "file.h"
#include "color.h"
#include "string.h"
#include "system_console.h"
#include "memutil.h"

HOSstatus BitmapToRGBArray(const char* FileName, rgb_t*** Result, rectangle_t* OutSize)
{
    file_object_t file;
    HOSstatus status = OpenFile(FileName, &file);
    if(HOS_ERROR(status))
    {
        return status;
    }

    bitmapfileheader_t bf = *(bitmapfileheader_t*)file.Buffer;
    bitmapinfoheader_t bi = *(bitmapinfoheader_t*)((size_t)file.Buffer + sizeof(bitmapfileheader_t));
    if (bf.bfType[0] != 'B' || bf.bfType[1] != 'M' || bi.biCompression != BI_RGB || bi.bcBitCount != 32 || bi.bcHeight <= 0)
    {
        CloseFile(&file);
        return STATUS_UNSUPPORTED;
    }

    i32 height = bi.bcHeight;
    u32 width = bi.bcWidth;
    OutSize->Height = bi.bcHeight;
    OutSize->Width = bi.bcWidth;
    size_t drawPx = 0;
    *Result = Alloc(sizeof(rgb_t*) * width);
    // TODO: Alloc失敗時のエラーハンドリングを実装
    for(u32 i=0; i<width; i++)
    {
        (*Result)[i] = Alloc(sizeof(rgb_t) * height);
        //TODO: Alloc失敗時のエラーハンドリングを実装
        for(i32 k= height -1; k >= 0; k--)
        {
            rgbquad_t q = *((rgbquad_t *)((size_t)file.Buffer + bf.bfOffBits) + drawPx++);
            (*Result)[k][i] = RGB(q.rgbRed, q.rgbGreen, q.rgbBlue);
        }
    }

    return STATUS_SUCCESS;
}

HOSstatus DrawBitmapToBuffer(const char* FileName, coordinate_t Location, rectangle_t* Rect)
{
    file_object_t file;
    HOSstatus status = OpenFile(FileName, &file);
    if(HOS_ERROR(status))
    {
        if(HOS_ERROR(status))
        {
#ifdef DEBUG
            puts("DrawBitmapToBuffer() error at OpenFile(): ");
            puts(utos(status));
            puts("\r\n");
#endif
            return status;
        }
        return status;
    }
    bitmapfileheader_t bf = *(bitmapfileheader_t*)file.Buffer;
    bitmapinfoheader_t bi = *(bitmapinfoheader_t*)((size_t)file.Buffer + sizeof(bitmapfileheader_t));
    if(bf.bfType[0] != 'B' || bf.bfType[1] != 'M' || bi.biCompression != BI_RGB || bi.bcBitCount != 32 || bi.bcHeight <= 0)
    {
        CloseFile(&file);
        return STATUS_UNSUPPORTED;
    }
    i32 height = bi.bcHeight;
    u32 width = bi.bcWidth;
    size_t drawPx = 0;
    for(i32 y=height - 1; y>=0; y--)
    {
        for(u32 x = 0; x<width; x++)
        {
            rgbquad_t q = *((rgbquad_t*)((size_t)file.Buffer + bf.bfOffBits)+drawPx++);
            DrawPixelToBuffer(COORD(Location.X+x, Location.Y+y), RGB(q.rgbRed, q.rgbGreen, q.rgbBlue));
        }
    }

    if(Rect != NULL)
    {
        *Rect = RECT(width, height);
    }
    CloseFile(&file);
    return STATUS_SUCCESS;
}

HOSstatus DrawBitmap(const char* FileName, coordinate_t Location, rectangle_t* Rect)
{
    HOSstatus status = DrawBitmapToBuffer(FileName, Location, Rect);
    DrawBufferContentsToFrameBuffer();
    return status;
}

rectangle_t GetPictureSize(const char* FileName)
{
    file_object_t file;
    HOSstatus status = OpenFile(FileName, &file);
    if(HOS_ERROR(status))
    {
        return RECT(0, 0);
    }
    bitmapfileheader_t bf = *(bitmapfileheader_t*)file.Buffer;
    bitmapinfoheader_t bi = *(bitmapinfoheader_t*)((size_t)file.Buffer + sizeof(bitmapfileheader_t));
    if(bf.bfType[0] != 'B' || bf.bfType[1] != 'M' || bi.biCompression != BI_RGB || bi.bcBitCount != 32 || bi.bcHeight <= 0)
    {
        CloseFile(&file);
        return RECT(0, 0);
    }
    return RECT(bi.bcWidth, bi.bcHeight);
}