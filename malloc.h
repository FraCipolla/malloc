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


#if __STDC_VERSION__ != 202311L
    #define false 0 
    #define true !false
    #define nullptr NULL
    #include <stdbool.h>
#endif

#define MALLOC_PAGE_SIZE	sysconf(_SC_PAGESIZE)

#define MAP_ANONYMOUS 0x20  /* Don't use a file. */
#define ALIGNMENT (sizeof(size_t))
#define ALIGN(size) (((size) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))
#define HEADER_ALIGN() (((sizeof(header_t)) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))
#define BLOCK_ALIGN() (((sizeof(block_t)) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#define TINY 1024
#define SMALL 4096
#define LARGE INT64_MAX
#define TYPE(size) (size )

#define TYPE_TO_SIZE(T) (T == 0 ? TINY : T == 1 ? SMALL : LARGE)

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
    header->offset = HEADER_ALIGN();                                            \
    header->type = t == E_TINY ? 0 : t == E_SMALL ? 1 : 2;                      \
    header->full = t == E_LARGE ? true : false;                                 \
    header->free = 0;                                                           \
    header->chunk_cap = alloc_size;                                             \
    return ptr;                                                                 \
}                     

#define _MALLOC(size, t, ptr)                                                   \
    pthread_mutex_lock(&g_mutex);                                               \
    if (!ptr || ((header_t *)ptr)->full) {                                      \
        void *p = init(size, t);                                                \
        if (!p) { return nullptr; }                                             \
    }                                                                           \
    header_t *header = ptr;                                                     \
    if (header->offset == HEADER_ALIGN() && header->type != 2) {                \
        ((block_t *)((char *)ptr + ALIGN(header->offset)))->prev = nullptr;     \
        ((block_t *)((char *)ptr + ALIGN(header->offset)))->first = 1;          \
    }                                                                           \
    block_t *block = (block_t *)((char *)ptr + ALIGN(header->offset));          \
    header->max_blocks++;                                                       \
    block->size = size;                                                         \
    block->used = 1;                                                            \
    block->type = t;                                                            \
    block->free = 0;                                                            \
    block->last = 1;                                                            \
    block->next = nullptr;                                                      \
    header->offset += header->type != 2 ? size + BLOCK_ALIGN() : 0;             \
    if (block->prev) {                                                          \
        (block->prev)->last = 0;                                                \
        (block->prev)->next = block;                                            \
    }                                                                           \
    if (header->type != 2) {                                                    \
        ((block_t *)((char *)ptr + ALIGN(header->offset)))->prev = block;       \
    }                                                                           \
    if ((header->offset + BLOCK_ALIGN() + (t == 0 ? TINY : t == 1 ? SMALL : 0)  \
        > header->chunk_cap && t != 2)) {                                       \
        header->full = 1;                                                       \
    }                                                                           \
    pthread_mutex_unlock(&g_mutex);                                             \
    return ((void *)((char *)block + BLOCK_ALIGN()))                         

#define DEALLOC(header, size, ptr)                                                                      \
    if (header->prev && header->next) {                                                                 \
        ((header_t *)(header->prev))->next = header->next;                                              \
    } else if (header->next) {                                                                          \
        header = header->next;                                                                          \
    } else if (header->prev) {                                                                          \
        ((header_t *)(header->prev))->next = nullptr;                                                   \
        header = header->prev;                                                                          \
    } else {                                                                                            \
        ptr = nullptr;                                                                                  \
    }                                                                                                   \
    munmap((char *)header, size);

#define SHOW_ALLOC_MEMORY(small, medium, large)                                                         \
    _SHOW_ALLOC_MEMORY(small, TINY);                                                                    \
    _SHOW_ALLOC_MEMORY(medium, SMALL);                                                                  \
    _SHOW_ALLOC_MEMORY_LARGE(large)

#define _SHOW_ALLOC_MEMORY(ptr, t) {                                                                    \
    if (ptr) {                                                                                          \
        void *list = ptr;                                                                               \
        while (((header_t *)list)->prev) {                                                              \
            list = ((header_t *)list)->prev;                                                            \
        }                                                                                               \
        println("%s : %p", #t, list);                                                                   \
        void *first_chunk = list;                                                                       \
        list = ((char *)list + HEADER_ALIGN());                                                         \
        while (42) {                                                                                    \
            if (((block_t *)list)->last) {                                                              \
                if (((block_t *)list)->used) {                                                          \
                    println(                                                                            \
                        "%p - %p : %d bytes",                                                           \
                        (char *)list + sizeof(block_t),                                                 \
                        (char *)list + ((block_t *)list)->size + sizeof(block_t),                       \
                        ((block_t *)list)->size                                                         \
                        );                                                                              \
                }                                                                                       \
                if (((header_t *)(first_chunk))->next) {                                                \
                    list = ((header_t *)(first_chunk))->next;                                           \
                    first_chunk = ((header_t *)(first_chunk))->next;                                    \
                    println("%s : %p",#t, list);                                                        \
                    list = ((char *)list + HEADER_ALIGN());                                             \
                } else {                                                                                \
                    break;                                                                              \
                }                                                                                       \
            }                                                                                           \
            if(((block_t *)list)->used && !((block_t *)list)->free) {                                   \
                println(                                                                                \
                    "%p - %p : %d bytes",                                                               \
                    (char *)list + sizeof(block_t),                                                     \
                    (char *)list + ((block_t *)list)->size + sizeof(block_t),                           \
                    ((block_t *)list)->size                                                             \
                    );                                                                                  \
            }                                                                                           \
            list = (char *)list + ALIGN((((block_t *)list)->size + BLOCK_ALIGN()));                     \
        }                                                                                               \
    }                                                                                                   \
}

#define _SHOW_ALLOC_MEMORY_LARGE(ptr) {                                                                 \
    if (!ptr) return;                                                                                   \
    void *list = ptr;                                                                                   \
    while (((header_t *)list)->prev) {                                                                  \
        list = ((header_t *)list)->prev;                                                                \
    }                                                                                                   \
    while (list) {                                                                                      \
        println("LARGE : %p", list);                                                                      \
        println(                                                                                        \
            "%p - %p : %d bytes",                                                                    \
            (char *)list + HEADER_ALIGN(),                                                            \
            (char *)list + ((block_t *)((char *)list + HEADER_ALIGN()))->size,                                            \
            ((block_t *)((char *)list + HEADER_ALIGN()))->size                                                              \
            );                                                                                          \
        list = ((header_t *)list)->next;                                                                \
    }                                                                                                   \
}

typedef enum {
    E_TINY,
    E_SMALL,
    E_LARGE
}   E_TYPES;

/* Block header size = 21 bytes */
typedef struct block_s {
    uint8_t                             :2;     /* Paddings bits */
    uint8_t         type                :2;     /* Type of block */
    uint8_t         used                :1;     /* Whether the block is used */
    uint8_t         free                :1;     /* Whether the block is being freed after used */
    uint8_t         first               :1;     /* Whether the block is the first one */
    uint8_t         last                :1;     /* Whether the block is the last one */
    uint32_t        size                  ;     /* Block size */
    struct block_s  *prev                 ;     /* Mark the next block */
    struct block_s  *next                 ;     /* Mark the previous block */
} block_t;

/* Chunk header size = 64 */
typedef struct header_s {
    uint8_t                     :5;
    uint8_t     type            :2;
    uint8_t     full            :1;
    uint8_t     free            :1;
    uint8_t     free_blocks       ;
    uint8_t     max_blocks        ;
    size_t      offset;
    size_t      chunk_cap;
    void        *next;
    void        *prev;
}   header_t;

typedef struct s_chunks {
    void            *small;
    void            *medium;
    void            *large;
}   chunks;

extern chunks g_chunks;

void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void show_alloc_mem();
// void show_alloc_mem_ex();

void println(const char *fmt, ...);

#endif
