#include "commonlib.h"
#include "typelib.h"

HOSstatus DrawBitmap(const char*);

typedef struct
{
    char bfType[2];
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
}PACKED_STRUCTURE bitmapfileheader_t;

typedef struct
{
    u32 bcSize;
    u32 bcWidth;
    u32 bcHeight;
    u16 bcPlanes;
    u16 bcBitCount;
    u32 biCompression;
    u32 biSizeImage;
    u32 biXPixPerMeter;
    u32 biYPixPerMeter;
    u32 biClrUsed;
    u32 biCirImportant;
}PACKED_STRUCTURE bitmapinfoheader_t;

typedef struct
{
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
}PACKED_STRUCTURE rgbquad_t;