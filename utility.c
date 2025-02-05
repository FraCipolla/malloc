#include "malloc.h"
#include <stdarg.h>

/* memcpy, should be almost always aligned */
void *ft_memcpy(void *dst, const void *src, size_t len)
{
    size_t i;
    unsigned long int *longword_ptr;
    unsigned long int longword, himagic, lomagic;

    /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
    himagic = 0x80808080L; // 10000000100000001000000010000000
    lomagic = 0x01010101L; // 1000000010000000100000001
    /* 64-bit version of the magic.  */
    /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
    if (sizeof (longword) > 4) {
        himagic = ((himagic << 16) << 16) | himagic;
        lomagic = ((lomagic << 16) << 16) | lomagic;
    }
    /* if aligned copy 1 word at a time */
    if ((uintptr_t)dst % sizeof(long) == 0 && (uintptr_t)src % sizeof(long) == 0 && len % sizeof(long) == 0) {
        longword_ptr = (unsigned long int *)dst;
        const unsigned long int *s = (unsigned long int *)src;
        for (i = 0; i < len / sizeof(unsigned long int); i++) {
            longword_ptr[i] = s[i];
            longword = longword_ptr[i];
            if (((longword - lomagic) & ~longword & himagic) != 0) {
                return dst;
            }
        }
    } else {
        char *d = dst;
        const char *s = src;
        for (i = 0; i < len; i++) {
            d[i] = s[i];
            if (d[i] == 0) {
                return dst;
            }
        }
    }
    return dst;
}

size_t ft_strlen(const char* s)
{
    const char *cpy = s;
    while (*s) {
        s++;
    }
    return (ptrdiff_t)(cpy - s);
}

/* simplified printf */
void print(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *str = "0123456789abcdef";
    int i = 0;
    char buff[128];
    
    while (*fmt) {
        if (*fmt == '%') {
            switch (*(++fmt)) {
            case 's':              /* string */
                char *s = va_arg(ap, char *);
                while(*s) {
                    write(1, s++, 1);
                };
                break;
            case 'd':              /* int */
                int d = va_arg(ap, int);
                if (d < 0) { write(1, "-", 1); d *= -1; }
                i = 0;
                while (d > 0) {
                    buff[i++] = (d % 10) + 48;
                    d /= 10;
                }
                while (i-- > 0) { write(1, &buff[i], 1); }
                break;
            case 'c':              /* char */
                char c = (char)va_arg(ap, int);
                write(1, &c, 1);
                break;
            case 'p':              /* pointer */
                long unsigned int p = va_arg(ap, long unsigned int);
                write(1, "0x", 2);
                i = 0;
                while (p > 0) {
                    buff[i] = str[p % 16];
                    p /= 16;
                    i++;
                }
                while (i-- > 0) { write(1, &buff[i], 1); }
                break;
            case 'x':
                long unsigned int x = va_arg(ap, long unsigned int);
                i = 0;
                while (x > 0) {
                    buff[i] = str[x % 16];
                    x /= 16;
                    i++;
                    if (i == 8) {
                        break;
                    }
                }
                while (i-- > 0) { write(1, &buff[i], 1); }
            }
        } else {
            write(1, fmt, 1);
        }
        fmt++;
    }
    va_end(ap);
}

void add_to_history(char * dest, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *str = "0123456789abcdef";
    int idx = 0, i = 0;
    char buff[128];
    
    while (*fmt) {
        if (*fmt == '%') {
            switch (*(++fmt)) {
            case 's':              /* string */
                char *s = va_arg(ap, char *);
                while(*s) {
                    dest[idx++] = *s;
                };
                break;
            case 'd':              /* int */
                int d = va_arg(ap, int);
                if (d < 0) { write(1, "-", 1); d *= -1; }
                i = 0;
                while (d > 0) {
                    buff[i++] = (d % 10) + 48;
                    d /= 10;
                }
                while (i-- > 0) { dest[idx++] = buff[i]; }
                break;
            case 'c':              /* char */
                char c = (char)va_arg(ap, int);
                dest[idx++] = c;
                break;
            case 'p':              /* pointer */
                long unsigned int p = va_arg(ap, long unsigned int);
                write(1, "0x", 2);
                i = 0;
                while (p > 0) {
                    buff[i] = str[p % 16];
                    p /= 16;
                    i++;
                }
                while (i-- > 0) { dest[idx++] = buff[i]; }
                break;
            case 'x':
                long unsigned int x = va_arg(ap, long unsigned int);
                i = 0;
                while (x > 0) {
                    buff[i] = str[x % 16];
                    x /= 16;
                    i++;
                    if (i == 8) {
                        break;
                    }
                }
                while (i-- > 0) { dest[idx++] = buff[i]; }
            }
        } else {
            dest[idx++] = *fmt;
        }
        fmt++;
    }
    va_end(ap);
}