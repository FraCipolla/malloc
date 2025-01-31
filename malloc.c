#include "malloc.h"
#include <stdarg.h>

chunks              g_chunks = {0};
pthread_mutex_t		g_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    if (size == 0) {
        return nullptr;
    }
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
        pthread_mutex_unlock(&g_mutex);
        return nullptr;
    } else if (size == 0) {
        ptr = nullptr;
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    }

    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());
    if (size <= cast->size || cast->extra_size + cast->size >= size) {
        cast->extra_size = cast->size + cast->extra_size - size; 
        cast->size = size;
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    } else if (!cast->next && cast->type != 2) {
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
        free(ptr);
        return new;
    }
    pthread_mutex_unlock(&g_mutex);
    return ptr;
}

/*
 * free the passed pointer. If it's the last pointer, delete it and move 1 block back.
 * If the block has adjacent free blocks, merge them to avoid fragmentation.
 */
void free(void *ptr)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        pthread_mutex_unlock(&g_mutex);
        return ;
    }

    ptr = (char *)ptr - BLOCK_ALIGN();
    block_t *cast = ptr;
    header_t *head = \
        cast->type == 0 ? g_chunks.small :  \
            cast->type == 1 ? g_chunks.medium : (ptr - HEADER_ALIGN());
    cast->used = 0;
    size_t size = 0;
    if (cast->type != 2 && !cast->next) {
        if (cast->prev) {
            cast->prev->extra_size += cast->size + cast->extra_size + BLOCK_ALIGN();
            cast->prev->next = nullptr;
        }
        head->offset = head->offset - (cast->size + cast->extra_size + BLOCK_ALIGN());
        // cast->extra_size = 0;
        // cast->size = 0;
        head->max_blocks--;
    } else if (cast->type != 2 && cast->next && !((cast->next)->used)) {
        head->max_blocks--;
        cast->extra_size += cast->next->size + cast->next->extra_size + BLOCK_ALIGN();
        cast->next = cast->next->next;
    } else if (cast->type != 2 && cast->prev && !((cast->prev)->used)) {
        head->max_blocks--;
        cast->next->prev = cast->prev;
        (cast->prev)->extra_size += cast->size + cast->extra_size + BLOCK_ALIGN();
        (cast->prev)->next = cast->next;
    } else {
        cast->used = 0;
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
