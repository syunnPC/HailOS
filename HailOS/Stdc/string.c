/*
    **AIによる自動生成**
    C 標準ライブラリ <string.h>
    作成日 2025-06-01
*/

#include "string.h"
#include "basetype.h"
#include "common.h" // Assuming COMMON_H might contain NULL definition if not in basetype.h
#include "memutil.h" // For MemEq, MemCopy, Alloc

size_t strlen(const char* Str)
{
    if(Str == NULL)
    {
        return 0; // Custom behavior: return 0 for NULL pointer
    }
    size_t result = 0;
    while(Str[result] != '\0')
    {
        result++;
    }
    return result;
}

// Note: This function checks for equality of up to 'Count' characters.
// It returns 0 if equal, 1 if not equal.
// It returns -1 if either Str1 or Str2 is NULL.
// This is different from standard C strncmp which performs lexicographical comparison.
int strncmp(const char* Str1, const char* Str2, size_t Count)
{
    if (Count == 0) {
        return 0; // 比較する文字数が0なら常に等しい
    }
    if (Str1 == NULL && Str2 == NULL) return 0; // 両方NULLなら等しい (カスタム動作)
    if (Str1 == NULL) return -1; // Str1がNULLなら小さい (カスタム動作)
    if (Str2 == NULL) return 1;  // Str2がNULLならStr1が大きい (カスタム動作)

    while (Count > 0 && *Str1 != '\0' && *Str1 == *Str2) {
        Str1++;
        Str2++;
        Count--;
        if (*(Str1-1) == '\0') { // 前の文字がヌル文字なら、両方ヌルで終わった場合のみ等しい
             break; 
        }
    }

    if (Count == 0) { // 指定された文字数分すべて一致した
        return 0;
    }
    
    // 比較が途中で終わった場合 (不一致または片方がヌル文字)
    return *(const unsigned char*)Str1 - *(const unsigned char*)Str2;
}


char* strstr(const char* Haystack, const char* Needle)
{
    if (Haystack == NULL || Needle == NULL)
    {
        return NULL;
    }

    size_t haystack_len = strlen(Haystack);
    size_t needle_len = strlen(Needle);

    if (needle_len == 0)
    {
        return (char*)Haystack; // Standard behavior: empty needle matches at start of haystack
    }

    if (needle_len > haystack_len)
    {
        return NULL; // Needle cannot be longer than haystack
    }

    for (size_t i = 0; i <= haystack_len - needle_len; i++)
    {
        // Use MemEq to compare 'needle_len' characters from Haystack[i] with Needle
        if (MemEq(&Haystack[i], Needle, needle_len))
        {
            return (char*)&Haystack[i]; // Match found
        }
    }

    return NULL; // No match found
}

char* itos(i64 n)
{
    char buf[21]; // Buffer for digits (reversed), sign, and null terminator
    int i = 0;    // Current index in buf
    bool is_negative = false;
    char* result;
    u64 u_n; // Use unsigned type for calculations to handle LLONG_MIN correctly

    if (n == 0)
    {
        result = Alloc(2);
        if (result == NULL) return NULL; // Check allocation result
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    if (n < 0)
    {
        is_negative = true;
        if (n == (-9223372036854775807LL - 1LL)) // LLONG_MIN
        {
            // Directly copy the digits of LLONG_MIN's absolute value
            const char* llong_min_abs_str = "9223372036854775808";
            size_t len = strlen(llong_min_abs_str);
            MemCopy(buf, llong_min_abs_str, len);
            i = len; // IMPORTANT: Update current index 'i'
            u_n = 0; // Mark as processed, so the digit extraction loop is skipped
        }
        else
        {
            u_n = (u64)(-n); // Convert to positive using u64
        }
    }
    else
    {
        u_n = (u64)n;
    }

    if (u_n > 0) // If u_n is not 0 (i.e., n was not LLONG_MIN which set u_n=0)
    {
        while (u_n > 0)
        {
            buf[i++] = (u_n % 10) + '0';
            u_n /= 10;
        }
    }

    if (is_negative)
    {
        buf[i++] = '-';
    }

    result = Alloc(i + 1); // Allocate exact memory needed (+1 for null terminator)
    if (result == NULL) return NULL; // Check allocation result

    // Reverse the string from buf into result
    for (int j = 0; j < i; j++)
    {
        result[j] = buf[i - 1 - j];
    }
    result[i] = '\0'; // Null terminate

    return result;
}

char* utos(u64 n)
{
    char buf[21]; // Buffer for digits (reversed) and null terminator. Max 20 digits for u64.
    int i = 0;    // Current index in buf
    char* result;

    if (n == 0)
    {
        result = Alloc(2);
        if (result == NULL) return NULL; // Check allocation result
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    while (n > 0)
    {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }

    result = Alloc(i + 1); // Allocate exact memory needed (+1 for null terminator)
    if (result == NULL) return NULL; // Check allocation result

    // Reverse the string from buf into result
    for (int j = 0; j < i; j++)
    {
        result[j] = buf[i - 1 - j];
    }
    result[i] = '\0'; // Null terminate

    return result;
}

static const char hex_chars[] = "0123456789ABCDEF";

char* ByteToHexString(u8 Value) 
{
    char* buffer = Alloc(3);
    if (buffer == NULL) 
    {
        return NULL;
    }
    buffer[0] = hex_chars[(Value >> 4) & 0x0F];
    buffer[1] = hex_chars[Value & 0x0F];
    buffer[2] = '\0';
    return buffer;
}