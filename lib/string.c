#include "string.h"

void *memset(void *dst, int c, uint64 n) {
    char *cdst = (char *)dst;
    for (uint64 i = 0; i < n; ++i)
        cdst[i] = c;

    return dst;
}
void *memmove(void *dst, const void *src, uint n)//从src复制n个到dst
{
    char *cdst = (char *)dst;
    char *csrc = (char *)src;
    for (uint64 i = 0; i < n; ++i)
        cdst[i] = csrc[i];

    return dst;
}
