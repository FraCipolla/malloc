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
    DEBUG("init\n");
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

    pthread_mutex_lock(&g_mutex);
    block_t *cast = (block_t *)((char *)ptr - ALIGN(sizeof(block_t)));
    void *new = malloc(size);
    for (long unsigned int i = 0; i < cast->size + ALIGN(sizeof(block_t)); i++) {
        *(char *)(new + i) = *(char *)(ptr + i);
    }
    free(ptr);
    ((block_t *)new)->size = size;
    pthread_mutex_unlock(&g_mutex);
    return ((void *)((char *)new + ALIGN(sizeof(block_t))));
}

void free(void *ptr)
{
    if (!ptr) { return ;}

    pthread_mutex_lock(&g_mutex);
    ptr = (char *)ptr - ALIGN(sizeof(block_t));
    block_t *cast = ptr;
    block_t *start = cast;
    cast->free = 1;
    size_t size = 0;
    // if (cast->prev) {
    //     block_t *next = (block_t *)((char *)cast + ALIGN(sizeof(cast->size)));
    //     next->prev = cast->prev; 
    // }
    while (cast && cast->prev) {
        cast = cast->prev;
    }
    // memset(cast, 0, ALIGN(sizeof(block_t)) + ALIGN(sizeof(cast->size)));
    header_t *header = (header_t *)((char *)cast - ALIGN(sizeof(header_t)));
    if ((char *)header + ALIGN(header->offset) == (char *)ptr + ALIGN(start->size + sizeof(block_t))) {
        println("before %d", header->offset);
        header->offset = header->offset - ALIGN(sizeof(start->size + sizeof(block_t)));
        println("after %d", header->offset);
        memset(start, 0, ALIGN(sizeof(block_t)) + ALIGN(sizeof(cast->size)));
        // header->free--;
    } else {
        header->free_blocks++;
    }
    
    if (header->full && header->free_blocks == header->max_blocks) {
        size = header->chunk_cap;
        switch (cast->type)
        {
        case E_TINY: DEALLOC(header, size, g_chunks.small); break;
        case E_SMALL: DEALLOC(header, size, g_chunks.medium); break;
        default: DEALLOC(header, size, g_chunks.large); break;
        }
    }
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_memory()
{
    pthread_mutex_lock(&g_mutex);
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

// void hex_dump(void *ptr, size_t size) {
//     unsigned char *byte_ptr = (unsigned char *)ptr;
//     for (size_t i = 0; i < size; i++) {
//         // Print each byte as two hexadecimal characters
//         printf("%02x ", byte_ptr[i]);

//         // Print a newline every 16 bytes for better readability
//         if ((i + 1) % 16 == 0) {
//             printf("\n");
//         }
//     }
//     printf("\n");
// }
