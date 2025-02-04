#include "malloc.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

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
    pthread_mutex_lock(&g_mutex);
    void *return_ptr = nullptr;
    if (size <= (TINY - BLOCK_ALIGN())) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - BLOCK_ALIGN())) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {
        _MALLOC(size, E_LARGE, g_chunks.large);
    }
    pthread_mutex_unlock(&g_mutex);
    return return_ptr;
}

void *calloc(size_t nmemb, size_t size)
{
    pthread_mutex_lock(&g_mutex);
    void *return_ptr = nullptr;
    size_t el_size = size;
    size = size * nmemb;
    if (size <= (TINY - BLOCK_ALIGN())) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - BLOCK_ALIGN())) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {
        _MALLOC(size, E_LARGE, g_chunks.large);
    }
    
    switch (el_size)
    {
    case 2:
        uint16_t *p16 = return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p16[i] = 0;
        }
        break;
    case 4:
        uint32_t *p32 = (uint32_t *)return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p32[i] = 0;
        }
        break;
    case 8:
        uint64_t *p64 = return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p64[i] = 0;
        }
        break;
    default:
        uint8_t *p = return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p[i] = 0;
        }
        break;
    }
    // sizeof(uint64_t) == 8
    pthread_mutex_unlock(&g_mutex);
    return return_ptr;
}

/*
* The  reallocarray()  function changes the size of the memory block pointed to by ptr to be large enough for an
* array of nmemb elements, each of which is size bytes.  It is equivalent to the call
* 
*   realloc(ptr, nmemb * size);
* 
* However, unlike that realloc() call, reallocarray() fails safely in the case where  the  multiplication  would
* overflow.  If such an overflow occurs, reallocarray() returns NULL, sets errno to ENOMEM, and leaves the orig‐
* inal block of memory unchanged.
*/
void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
    long long x = nmemb * size;
    if ((nmemb != 0 && size != 0) && nmemb != x / size) {
        // overflow handling
        errno = ENOMEM;
        return nullptr;
    }
    return realloc(ptr, nmemb * size);
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
        g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Attempt to realloc a null pointer: %p\n", ptr);
        pthread_mutex_unlock(&g_mutex);
        return nullptr;
    } else if (size == 0) {
        g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Reallocing pointer giving 0 size (equale free): %p\n", ptr);
        _FREE(ptr)
        ptr = nullptr;
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    }

    g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Reallocing pointer: %p for size: %ld\n", ptr, size);
    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());
    if (size <= cast->size || cast->extra_size + cast->size >= size) {
        g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "   Enought size in block %p, no need to realloc\n", ptr);
        cast->extra_size = cast->size + cast->extra_size - size; 
        cast->size = size;
    } else if (!cast->next && cast->type != 2) {
        g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "   Pointer %p is the last allocated block of the chunk, no need to realloc\n", ptr);
        cast->size = size;
        cast->extra_size = ALIGN(size);
    } else {
        g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "   Not enought size (%ld) for pointer %p, allocate a new pointer and free the old one\n", size, ptr);
        void *return_ptr = nullptr;
        if (size <= (TINY - BLOCK_ALIGN())) {
            _MALLOC(size, E_TINY, g_chunks.small);
        } else if (size <= (SMALL - BLOCK_ALIGN())) {
            _MALLOC(size, E_SMALL, g_chunks.medium);
        } else {
            _MALLOC(size, E_LARGE, g_chunks.large);
        }
        ft_memcpy(return_ptr, (void *)cast + BLOCK_ALIGN(), cast->size);
        _FREE(ptr)
        return return_ptr;
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
        g_chunks.history.idx += sprintf(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Attempt to free a null pointer: %p\n", ptr);
        pthread_mutex_unlock(&g_mutex);
        return ;
    }

    _FREE(ptr)
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_mem()
{
    pthread_mutex_lock(&g_mutex);
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_mem_ex()
{
    pthread_mutex_lock(&g_mutex);
    print("%s", g_chunks.history.buffer);
    pthread_mutex_unlock(&g_mutex);
}

void hex_dump() {
    pthread_mutex_lock(&g_mutex);
    HEX_DUMP(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

void print_memory() {
    pthread_mutex_lock(&g_mutex);
    PRINT_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}
