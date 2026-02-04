#include "string.h"

size_t strlen(const char* str) {
    if (str == NULL) {
        return 0;
    }

    size_t len = 0;
    while (str[len]) {
        len++;
    }

    return len;
}

char* strncpy(char *dst, const char *src, size_t count) {
    if (count == 0) {
        return dst;
    }
    char* origDst = dst;

    if (dst == NULL) {
        return NULL;
    }

    if (src == NULL) {
        *dst = '\0';
        return dst;
    }

    while (*src) {
        *dst = *src;
        src++;
        dst++;
        if (--count <= 0) {
            return origDst;
        }
    }
    
    *dst = '\0';
    return origDst;
}

const char* strchr(const char* str, char chr)
{
    if (str == NULL)
        return NULL;

    while (*str)
    {
        if (*str == chr)
            return str;

        ++str;
    }

    return NULL;
}

char* strcpy(char* dst, const char* src)
{
    char* origDst = dst;

    if (dst == NULL)
        return NULL;

    if (src == NULL)
    {
        *dst = '\0';
        return dst;
    }

    while (*src)
    {
        *dst = *src;
        ++src;
        ++dst;
    }
    
    *dst = '\0';
    return origDst;
}

bool islower(char chr) {
    return chr >= 'a' && chr <= 'z';
}

char toupper(char chr) {
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}