#include "malloc.h"
#include <stdarg.h>

chunks              g_chunks = {0};
pthread_mutex_t		g_mutex = PTHREAD_MUTEX_INITIALIZER;

/* memcpy, should be almost always aligned */
void *ft_memcpy(void *dst, const void *src, size_t len)
{
    size_t i;

    /* if aligned copy 1 word at a time */
    if ((uintptr_t)dst % sizeof(long) == 0 && (uintptr_t)src % sizeof(long) == 0 && len % sizeof(long) == 0) {
        long *d = dst;
        const long *s = src;
        for (i = 0; i < len / sizeof(long); i++) {
            d[i] = s[i];
        }
    } else {
        char *d = dst;
        const char *s = src;
        for (i = 0; i < len; i++) {
            d[i] = s[i];
        }
    }
    return dst;
}

/* simplified printf */
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
            }
        } else {
            write(1, fmt, 1);
        }
        fmt++;
    }
    va_end(ap);
    write(1, "\n", 1);
}

/* init memory zones */
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

/* allocate (2 * sizeof(size_t)) aligned memory block */
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

/* 
 * realloc passed pointer. If no block after the passed one exists
 * or if the block extra size is enought, set the new size and return the pointer.
 * Else, call malloc for a new pointer, copy the old pointer to the new one, and free the old pointer
*/
void *realloc(void *ptr, size_t size)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        return nullptr;
    }

    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());
    if (size <= cast->size || cast->extra_size + cast->size >= size) {
        cast->extra_size = cast->size + cast->extra_size - size; 
        cast->size = size;
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
        ft_memcpy(new, (void *)cast + BLOCK_ALIGN(), cast->size);
        pthread_mutex_unlock(&g_mutex);
        // fix free between blocks
        free(ptr);
        return new;
    }
    return ptr;
}

/*
 * free the passed pointer. If it's the last pointer, delete it and move 1 block back.
 * If the block has adjacent free blocks, merge them to avoid fragmentation.
 */
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
    } else if (cast->type != 2 && (cast->next)->used && (cast->next)->free) {
        head->max_blocks--;
        cast->extra_size += cast->next->size + cast->next->extra_size + BLOCK_ALIGN();
        cast->next = cast->next->next;
    }else if (cast->type != 2 && (cast->prev)->used && (cast->prev)->free) {
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