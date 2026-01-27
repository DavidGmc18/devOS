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