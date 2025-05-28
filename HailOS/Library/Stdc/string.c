#include "string.h"
#include "typelib.h"
#include "commonlib.h"
#include "memmgr.h"
#include "util.h"
#include "status.h"

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

    for(size_t i=0; i<Size; i++)
    {
        ((u8*)Dest)[i] = ((u8*)Src)[i];
    }

    return Dest;
}

boolean memeq(const void* Buf1, const void* Buf2, size_t BufSize)
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

    return memeq(Str1, Str2, lenStr1);
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

    return memeq(Str1, Str2, Count);
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

char* nstd_strcat_s(const char* Str1, const char* Str2)
{
    if(Str1 == NULL || Str2 == NULL)
    {
        return NULL;
    }

    char* result = KernelAlloc(strlen(Str1)+strlen(Str2)+1);
    if(result == NULL)
    {
        return NULL;
    }

    memcpy(result, Str1, strlen(Str1));
    memcpy(result+strlen(Str1), Str2, strlen(Str2)+1);

    return result;
}

char* itos(i64 n)
{
    char* result;
    char buf[21];
    int i=0;
    boolean isNegative = false;

    if(n == 0)
    {
        result = KernelAlloc(2);
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    if(n < 0)
    {
        isNegative = true;
        if(n == -9223372036854775807-1)
        {
            const char* min = "9223372036854775808";
            memcpy(buf, min, strlen(min));
        }
        else
        {
            n = -n;
        }
    }

    if(n != -9223372036854775807-1)
    {
        while(n > 0)
        {
            buf[i++] = (n  % 10) + '0';
            n /= 10;
        }
    }

    if(isNegative)
    {
        buf[i++] = '-';
    }

    result = KernelAlloc(21);

    for(int j=0; j<i; j++)
    {
        result[j] = buf[i-j-1];
    }
    result[i] = '\0';

    return result;
}

char* utos(u64 n)
{
    char* result;
    int i=0;
    char buf[21];

    if(n == 0)
    {
        result = KernelAlloc(2);
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    while(n > 0)
    {
        buf[i++] = (n  % 10) + '0';
        n /= 10;
    }

    result = KernelAlloc(21);

    for(int j=0; j<i; j++)
    {
        result[j] = buf[i-j-1];
    }
    result[i] = '\0';

    return result;
}