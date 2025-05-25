#include "string.h"
#include "typelib.h"
#include "commonlib.h"

size_t strlen(const char* Str)
{
    if(Str == NULL)
    {
        return 0;
    }

    size_t idx = 0;
    while(Str[idx] != '\0')
    {
        idx++;
    }

    return idx;
}

void* memcpy(void* Dest, const void* Src, size_t Size)
{
    if(Dest == NULL || Src == NULL)
    {
        return NULL;
    }

    if((size_t)Src + Size >= (size_t)Dest)
    {
        return NULL;
    }

    for(size_t i=0; i<Size; i++)
    {
        ((u8*)Dest)[i] = ((u8*)Src)[i];
    }

    return Dest;
}

boolean memcmp(const void* Buf1, const void* Buf2, size_t BufSize)
{
    if(BufSize == 0)
    {
        return true;
    }

    if(Buf1 == NULL || Buf2 == NULL)
    {
        return false;
    }

    for(size_t i=0; i<BufSize; i++)
    {
        if(((u8*)Buf1)[i] != ((u8*)Buf2)[i])
        {
            return false;
        }
    }

    return true;
}

boolean strcmp(const char* Str1, const char* Str2)
{

    if(Str1 == NULL || Str2 == NULL)
    {
        return false;
    }

    size_t lenStr1 = strlen(Str1);
    size_t lenStr2 = strlen(Str2);

    if(lenStr1 != lenStr2)
    {
        return false;
    }

    return memcmp(Str1, Str2, lenStr1);
}

void* memset(void* Dest, char Ch, size_t Count)
{
    if(Dest == NULL)
    {
        return NULL;
    }

    for(size_t i = 0; i<Count; i++)
    {
        ((char*)Dest)[i] = Ch;
    }

    return Dest;
}

boolean strncmp(const char* Str1, const char* Str2, size_t Count)
{

    if(Str1 == NULL || Str2 == NULL)
    {
        return false;
    }

    size_t lenStr1 = strlen(Str1);
    size_t lenStr2 = strlen(Str2);

    if(lenStr1 < Count || lenStr2 < Count)
    {
        Count = (lenStr1 < lenStr2 ? lenStr1 : lenStr2);
    }

    return memcmp(Str1, Str2, Count);
}

char* strstr(char* Str1, const char* Str2)
{
    char* result = NULL;
    if(strlen(Str1) < strlen(Str2))
    {
        return result;
    }

    boolean Success = true;

    for(size_t i=0; i<strlen(Str1); i++)
    {
        if(Str1[i] == Str2[0])
        {
            result = &Str1[i];
            if(i + strlen(Str2) - 1 > strlen(Str2))
            {
                return NULL;
            }

            for(size_t j=0; j<strlen(Str2); j++)
            {
                if(Str1[j+i] != Str2[j])
                {
                    Success = false;
                    break;
                }
            }

            if(Success)
            {
                return result;
            }
        }
    }

    return NULL;
}