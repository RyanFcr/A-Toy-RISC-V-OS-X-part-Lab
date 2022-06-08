#include "string.h"

#include "stdio.h"
#include "syscall.h"

char *gets(char *buf, int max)
{
    int i, cc;
    char c;

    for (i = 0; i + 1 < max;)
    {
        cc = read(0, &c, 1);
        // printf("%c\n", c);
        if (c < 0x20 || c > 0x7e)
            continue;
        if (cc == 0)
            continue;
        if (cc < 1)
            break;
        buf[i++] = c;
        if (c == '\n' || c == '\r')
            break;
    }
    buf[i - 1] = '\0';
    return buf;
}

void *memset(void *dst, int c, unsigned int n)
{
    char *cdst = (char *)dst;
    int i;
    for (i = 0; i < n; i++)
    {
        cdst[i] = c;
    }
    return dst;
}

int strcmp(const char *p, const char *q)
{
    while (*p && *p == *q)
        p++, q++;
    return (unsigned char)*p - (unsigned char)*q;
}