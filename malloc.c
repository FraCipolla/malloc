#include "malloc.h"
#include <stdarg.h>

chunks              g_chunks = {0};
pthread_mutex_t		g_mutex = PTHREAD_MUTEX_INITIALIZER;

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
                unsigned int x = va_arg(ap, long unsigned int);
                i = 0;
                while (x > 0) {
                    buff[i] = str[x % 16];
                    write(1, &buff[i], 1);
                    p /= 16;
                    i++;
                }
                switch (i)
                {
                case 0: write(1, "00", 1); break;
                case 1: write(1, "0", 1); write(1, &buff[0], 1); break;
                default: write(1, &buff[0], 2); break;
                }
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

static inline void* init(size_t size, E_TYPES type)
{
    switch (type)
    {
    case E_TINY: INIT(E_TINY, TINY, g_chunks.small);
    case E_SMALL: INIT(E_SMALL, SMALL, g_chunks.medium);
    default: INIT(E_LARGE, size, g_chunks.large);
    }
    return nullptr;
}

void *malloc(size_t size)
{
    if (size <= (TINY - BLOCK_ALIGN())) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - BLOCK_ALIGN())) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {
        _MALLOC(size, E_LARGE, g_chunks.large);
    }
}

void *realloc(void *ptr, size_t size)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        return nullptr;
    }

    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());
    if (size <= cast->size) { 
        return ptr;
    }
    else if (!cast->next && cast->type != 2) {
        header_t *head = cast->type == 0 ? g_chunks.small : cast->type == 1 ? g_chunks.medium : g_chunks.large;
        head->offset += size - cast->size;
        cast->size = size;
        cast->extra_size = ALIGN(size);
    } else {
        pthread_mutex_unlock(&g_mutex);
        void *new = malloc(size);
        pthread_mutex_lock(&g_mutex);
        for (long unsigned int i = 0; i < cast->size; i++) {
            *(char *)(new + i) = *(char *)(ptr + i + BLOCK_ALIGN());
        }
        // memcpy(new, (void *)cast + BLOCK_ALIGN(), cast->size);
        pthread_mutex_unlock(&g_mutex);
        // fix free between blocks
        free(ptr);
        return new;
    }
    return ptr;
}

void free(void *ptr)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) { return ;}

    ptr = (char *)ptr - BLOCK_ALIGN();
    block_t *cast = ptr;
    header_t *head = \
        cast->type == 0 ? g_chunks.small :  \
            cast->type == 1 ? g_chunks.medium : (ptr - HEADER_ALIGN());
    cast->free = 1;
    size_t size = 0;
    if (cast->type != 2 && !cast->next) {
        head->offset = head->offset - (cast->size + cast->extra_size + BLOCK_ALIGN());
        cast->used = 0;
        cast->extra_size = 0;
        cast->size = 0;
        cast->free = 1;
        head->max_blocks--;
    } else if (cast->type != 2 && (cast->prev)->used && (cast->prev)->free) {
        head->max_blocks--;
        (cast->prev)->extra_size += cast->size + cast->extra_size + BLOCK_ALIGN();
        (cast->prev)->next = cast->next;
    } else {
        head->free_blocks++;
    }

    if (head->full && head->free_blocks == head->max_blocks) {
        size = head->chunk_cap;
        switch (cast->type)
        {
        case E_TINY: DEALLOC(head, size, g_chunks.small); break;
        case E_SMALL: DEALLOC(head, size, g_chunks.medium); break;
        default: DEALLOC(head, size, g_chunks.large); break;
        }
    }
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_mem()
{
    pthread_mutex_lock(&g_mutex);
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

// void show_alloc_mem_ex()
// {
//     pthread_mutex_lock(&g_mutex);
//     SHOW_ALLOC_MEMORY_EX(g_chunks.small, g_chunks.medium, g_chunks.large);
//     pthread_mutex_unlock(&g_mutex);
// }