#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Mutex for thread safe malloc */
#include <pthread.h>
extern pthread_mutex_t	g_mutex;

/* define nullptr if c version < 23 */
#if __STDC_VERSION__ != 202311L
    #define false 0 
    #define true !false
    #define nullptr NULL
    #include <stdbool.h>
#endif

/* Malloc pagesize (4096 on 64bit machines) */
#define MALLOC_PAGE_SIZE	sysconf(_SC_PAGESIZE)

#define MAP_ANONYMOUS 0x20  /* Don't use a file. */
#define ALIGNMENT (2 * sizeof(size_t))
#define ALIGN(size) (((size) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))
#define HEADER_ALIGN() (((sizeof(header_t)) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))
#define BLOCK_ALIGN() (((sizeof(block_t)) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#define TTYPE(t) t == E_TINY ? "TINY" : t == E_SMALL ? "SMALL" : "LARGE"

#define TINY 512
#define SMALL 4096
#define LARGE INT64_MAX

#define INIT(t, size, ptr)                                                      \
{                                                                               \
    const size_t page =  sysconf(_SC_PAGESIZE);                                 \
    const size_t alloc_size =                                                   \
            t == E_TINY ? ((100 / (page / TINY)) + 2) * page :                  \
            t == E_SMALL ? ((100 / (page / SMALL)) + 2) * page :                \
            ((size + HEADER_ALIGN() + BLOCK_ALIGN()) / page + 1) * page;        \
    void *map = mmap(                                                           \
        NULL,                                                                   \
        alloc_size,                                                             \
        PROT_READ | PROT_WRITE,                                                 \
        MAP_PRIVATE | MAP_ANONYMOUS,                                            \
        -1,                                                                     \
        0                                                                       \
    );                                                                          \
    if (map == MAP_FAILED) { return nullptr; }                                  \
    if (ptr && ((header_t *)ptr)->full) {                                       \
        ((header_t *)map)->prev = ptr;                                          \
        ((header_t *)ptr)->next = map;                                          \
    } else {                                                                    \
        ((header_t *)map)->prev = nullptr;                                      \
        ((header_t *)map)->next = nullptr;                                      \
    }                                                                           \
    ptr = map;                                                                  \
    header_t *header = ptr;                                                     \
    header->max_blocks = 0;                                                     \
    header->free_blocks = 0;                                                    \
    header->full = t == E_LARGE ? true : false;                                 \
    header->free = 0;                                                           \
    header->chunk_cap = alloc_size;                                             \
    return ptr;                                                                 \
}

#define _MALLOC(size, t, ptr)                                                   \
    if (!ptr || ((header_t *)ptr)->full) {                                      \
        void *p = init(size, t);                                                \
        if (!p) {                                                               \
            pthread_mutex_unlock(&g_mutex);                                     \
            return nullptr;                                                     \
        }                                                                       \
    }                                                                           \
    header_t *header = ptr;                                                     \
    header->max_blocks++;                                                       \
    block_t *block = (block_t *)((char *)ptr + HEADER_ALIGN());                 \
    block->first = 1;                                                           \
    block_t *prev = block->used ? block : nullptr;                              \
    while (block->next && block->next->used                                     \
            && block->extra_size < (size + BLOCK_ALIGN())) {                    \
        block = block->next;                                                    \
        prev = block;                                                           \
    }                                                                           \
    if (block->next && block->used                                              \
        && block->extra_size >= (ALIGN(size) + BLOCK_ALIGN())) {                \
        block_t *next = block->next;                                            \
        prev = block;                                                           \
        block->extra_size = 0;                                                  \
        block =                                                                 \
            (block_t *)((char *)block + (block->size + BLOCK_ALIGN()));         \
        block->next = next;                                                     \
        block->extra_size =                                                     \
            block->prev->extra_size - (block->prev->size + BLOCK_ALIGN());      \
    } else if (block->used) {                                                   \
        block =                                                                 \
            (block_t *)                                                         \
            ((char *)block + block->size + block->extra_size + BLOCK_ALIGN());  \
        prev->next = block;                                                     \
    }                                                                           \
    if (prev) {                                                                 \
        prev->last = 0;                                                         \
    }                                                                           \
    block->prev = prev;                                                         \
    block->size = size;                                                         \
    /* Round request up to a multiple of 64 that is at least 64 */              \
    if (!block->extra_size) {                                                   \
        block->extra_size = t == E_LARGE ?                                      \
            header->chunk_cap - (size + BLOCK_ALIGN() + HEADER_ALIGN()) :       \
            ALIGN(size) - size;                                                 \
    }                                                                           \
    block->used = 1;                                                            \
    block->type = t;                                                            \
    block->last = 1;                                                            \
    block->next = nullptr;                                                      \
    if (t != 2 && (ptr + header->chunk_cap) - (void *)block <                   \
            (t == E_TINY ? TINY : t == E_SMALL ? SMALL : 0)                     \
        ) {                                                                     \
        header->full = 1;                                                       \
    }                                                                           \
    return_ptr = ((void *)((char *)block + BLOCK_ALIGN()))                      

#define _FREE(ptr)                                                              \
    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());                   \
    header_t *head =                                                            \
        cast->type == 0 ? g_chunks.small :                                      \
            cast->type == 1 ? g_chunks.medium : (ptr - HEADER_ALIGN());         \
    cast->used = 0;                                                             \
    head->max_blocks--;                                                         \
    size_t size = 0;                                                            \
    if (cast->type != 2) {                                                      \
        if (!cast->next && !cast->prev) {                                       \
        } else if (!cast->next) {                                               \
            cast->prev->extra_size +=                                           \
                 cast->size + cast->extra_size + BLOCK_ALIGN();                 \
            cast->prev->next = nullptr;                                         \
            if (cast->prev->first) {                                            \
                cast->prev->used = 0;                                           \
            }                                                                   \
        } else if (!cast->prev) {                                               \
            cast->used = 1;                                                     \
            cast->extra_size += cast->size;                                     \
            cast->size = 0;                                                     \
        } else {                                                                \
            if (cast->prev && cast->next) {                                     \
                cast->prev->extra_size +=                                       \
                    cast->size + cast->extra_size + BLOCK_ALIGN();              \
                cast->prev->next = cast->next;                                  \
                cast->next->prev = cast->prev;                                  \
            } else if (cast->prev) {                                            \
                cast->prev->extra_size +=                                       \
                    cast->size + cast->extra_size + BLOCK_ALIGN();              \
                cast->prev->next = nullptr;                                     \
            } else if (cast->next) {                                            \
                cast->extra_size += cast->size;                                 \
                cast->size = 0;                                                 \
            }                                                                   \
        }                                                                       \
        bzero(cast, cast->size + cast->extra_size + BLOCK_ALIGN());             \
    }                                                                           \
    if ((head->full && head->free_blocks == head->max_blocks)                   \
        || (cast && cast->type == 2)) {                                         \
        size = head->chunk_cap;                                                 \
        switch (cast->type)                                                     \
        {                                                                       \
        case E_TINY: DEALLOC(head, size, g_chunks.small, cast->type); break;    \
        case E_SMALL: DEALLOC(head, size, g_chunks.medium, cast->type); break;  \
        default: DEALLOC(head, size, g_chunks.large, cast->type); break;        \
        }                                                                       \
    }

#define DEALLOC(header, size, ptr, t)                                           \
    if (header->prev && header->next) {                                         \
        ((header_t *)(header->prev))->next = header->next;                      \
    } else if (header->next) {                                                  \
        header = header->next;                                                  \
    } else if (header->prev) {                                                  \
        ((header_t *)(header->prev))->next = nullptr;                           \
        header = header->prev;                                                  \
    } else {                                                                    \
        ptr = nullptr;                                                          \
    }                                                                           \
    munmap((char *)header, size);

#define SHOW_ALLOC_MEMORY(small, medium, large)                                 \
    _SHOW_ALLOC_MEMORY(small, TINY);                                            \
    _SHOW_ALLOC_MEMORY(medium, SMALL);                                          \
    _SHOW_ALLOC_MEMORY(large, LARGE)

#define HEX_DUMP(small, medium, large)                                          \
    _HEX_DUMP(small);                                                           \
    _HEX_DUMP(medium);                                                          \
    _HEX_DUMP(large); 

#define PRINT_MEMORY(small, medium, large)                                      \
    _PRINT_MEMORY(small, TINY);                                                 \
    _PRINT_MEMORY(medium, SMALL);                                               \
    _PRINT_MEMORY(large, LARGE); 

#define _SHOW_ALLOC_MEMORY(ptr, t) {                                            \
    if (ptr) {                                                                  \
        header_t *list = ptr;                                                   \
        while (list->prev) {                                                    \
            list = list->prev;                                                  \
        }                                                                       \
        while (list) {                                                          \
            print("%s : %p\n",#t, list);                                        \
            block_t *block = (block_t *)((char *)list + HEADER_ALIGN());        \
            size_t total_size = 0;                                              \
            while (block) {                                                     \
                if (block->used && !block->free) {                              \
                    print(                                                      \
                        "%p - %p : %d bytes\n",                                 \
                        (char *)block + BLOCK_ALIGN(),                          \
                        (char *)block + block->size + BLOCK_ALIGN(),            \
                        block->size                                             \
                        );                                                      \
                        total_size += block->size;                              \
                }                                                               \
                block = block->next;                                            \
            }                                                                   \
            print("Total : %d bytes\n\n", total_size);                          \
            list = list->next;                                                  \
        }                                                                       \
    }                                                                           \
}

#define PRINT_HEX_DUMP(ptr, size)                                               \
    unsigned char *data = (unsigned char*)ptr;                                  \
    char hex_digits[] = "0123456789ABCDEF";                                     \
    print("%x:  ", data);                                                       \
    for (size_t i = 0; i < size; i++) {                                         \
        char hex[3] =                                                           \
            { hex_digits[(data[i] >> 4) & 0x0F], hex_digits[data[i] & 0x0F]};   \
        print("%s ", hex);                                                      \
        if ((i + 1) % 16 == 0) {                                                \
            if (i + 1 < size) {                                                 \
                print("\n%x:  ", &data[i + 1]);                                 \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    print("\n\n");

#define _HEX_DUMP(ptr) {                                                        \
    if (ptr) {                                                                  \
        header_t *list = ptr;                                                   \
        while (list->prev) {                                                    \
            list = list->prev;                                                  \
        }                                                                       \
        while (list) {                                                          \
            block_t *block = (block_t *)((char *)list + HEADER_ALIGN());        \
            if (block && block->size > 0) {                                     \
                print("%p\n",list);                                             \
                while (block) {                                                 \
                    PRINT_HEX_DUMP(block, block->size);                         \
                    block = block->next;                                        \
                }                                                               \
            }                                                                   \
            list = list->next;                                                  \
        }                                                                       \
    }                                                                           \
}

// Used memory \u2588 █
// Empty memory \u2591 ░
// Chunk header \u2592 ▒
// Block header \u2593 ▓

#define _PRINT_MEMORY(ptr, t) {                                                 \
    if (ptr) {                                                                  \
        header_t *list = ptr;                                                   \
        while (list->prev) {                                                    \
            list = list->prev;                                                  \
        }                                                                       \
        while (list) {                                                          \
            block_t *block = (block_t *)((char *)list + HEADER_ALIGN());        \
            if (block && block->size > 0) {                                     \
                print("%s : %p\n",#t, list);                                    \
                size_t i = 0;                                                   \
                while (i < HEADER_ALIGN()) { print("\u2592"); ++i; }            \
                while (block) {                                                 \
                    for (size_t j = 0; j < BLOCK_ALIGN(); j++) {                \
                        if (i > 0 && i % 128 == 0) { print("\n"); }             \
                        print("\u2593");                                        \
                        ++i;                                                    \
                    }                                                           \
                    for (size_t j = 0; j < (block->size); j++) {                \
                        if (i > 0 && i % 128 == 0) { print("\n"); }             \
                        print("\u2588");                                        \
                        ++i;                                                    \
                    }                                                           \
                    for (size_t j = 0; j < (block->extra_size); j++) {          \
                        if (i > 0 && i % 128 == 0) { print("\n"); }             \
                        print("\u2591");                                        \
                        ++i;                                                    \
                    }                                                           \
                    block = block->next;                                        \
                }                                                               \
            }                                                                   \
            list = list->next;                                                  \
            print("\n");                                                        \
        }                                                                       \
    }                                                                           \
}

typedef enum {
    E_TINY,
    E_SMALL,
    E_LARGE
}   E_TYPES;

/* Block header size = 24 bytes */
typedef struct block_s {
    uint64_t            type                :2 ;   /* Type of block */
    uint64_t            used                :1 ;   /* Whether the block is used */
    uint64_t            free                :1 ;   /* Whether the block is being freed after used */
    uint64_t            first               :1 ;   /* Whether the block is the first one */
    uint64_t            last                :1 ;   /* Whether the block is the last one */
    uint64_t            size                :32;   /* Block size */
    uint64_t            extra_size          :26;   /* Total block capacity. Needed for defragmentation */
    struct block_s*     next                   ;   /* Mark the prev block*/
    struct block_s*     prev                   ;   /* Mark the prev block*/
} block_t;

/* Chunk header size = 64 */
typedef struct header_s {
    uint8_t                                 :6;
    uint8_t             full                :1;
    uint8_t             free                :1;
    uint8_t             free_blocks           ;
    uint8_t             max_blocks            ;
    size_t              chunk_cap;
    void                *next;
    void                *prev;
}   header_t;

typedef struct history_s {
    int     idx;
    char    buffer[8192 * 4];
}   history_t;

typedef struct chunks_s {
    void            *small;
    void            *medium;
    void            *large;
    history_t       history;
}   chunks;

extern chunks g_chunks;

void free(void *ptr);
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
void show_alloc_mem();
void show_alloc_mem_ex();
void hex_dump();
void print_memory();

/* utility */
void print(const char *fmt, ...);
void add_to_history(char * dest, const char *fmt, ...);
void *ft_memcpy(void *dst, const void *src, size_t len);
size_t ft_strlen(const char* s);

#endif