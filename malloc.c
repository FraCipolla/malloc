#include "malloc.h"
#include <stdarg.h>

chunks g_chunks = {0};

void println(const char *fmt, ...)
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
                if (d < 0) {
                    write(1, "-", 1);
                    d *= -1;
                }
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
            }
        } else {
            write(1, fmt, 1);
        }
        fmt++;
    }
    va_end(ap);
    write(1, "\n", 1);
}

static inline void init(size_t size)
{
    switch (size)
    {
    case TINY: INIT(TINY, g_chunks.small); break;
    case SMALL: INIT(SMALL, g_chunks.medium); break;
    default: INIT(size, g_chunks.large); break;
    }
}

void *malloc(size_t size)
{
    if (size <= (TINY - ALIGN(sizeof(block_t)))) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - ALIGN(sizeof(block_t)))) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {
        _MALLOC(size, E_LARGE, g_chunks.large);
    }
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) {
        return nullptr;
    }

    block_t *cast = (block_t *)((char *)ptr - ALIGN(sizeof(block_t)));
    void *new = malloc(size);
    memcpy(new, ptr, cast->size);
    free(ptr);
    return new;
}

void free(void *ptr)
{
    if (!ptr) { return ;}

    ptr = (char *)ptr - ALIGN(sizeof(block_t));
    block_t *cast = ptr;
    cast->free = 1;
    size_t size = 0;
    while (cast && cast->prev) {
        cast = cast->prev;
    }

    header_t *header = (header_t *)((char *)cast - ALIGN(sizeof(header_t)));
    header->free_blocks++;
    
    if (header->full && header->free_blocks == header->max_blocks) {
        size = header->chunk_cap;
        switch (cast->type)
        {
        case E_TINY: DEALLOC(header, size, g_chunks.small); break;
        case E_SMALL: DEALLOC(header, size, g_chunks.medium); break;
        default: DEALLOC(header, size, g_chunks.large); break;
        }
    }
}

void show_alloc_memory()
{
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
}