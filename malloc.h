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

#define DEBUG(x) write(1, x, strlen(x))

#define MAP_ANONYMOUS 0x20  /* Don't use a file. */
#define ALIGNMENT 64
#define ALIGN(size) (((size) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#define TINY 512
#define SMALL 2048
#define LARGE INT64_MAX

#define TYPE_TO_SIZE(T) (T == 0 ? TINY : T == 1 ? SMALL : LARGE)

#define INIT(size, ptr)                                                                                 \
{                                                                                                       \
    const size_t page =  sysconf(_SC_PAGESIZE);                                                         \
    const size_t alloc_size =                                                                           \
            (size == TINY || size == SMALL) ?                                                           \
            ((100 / (page / size)) + 1) * page :                                                        \
            ((size + ALIGN(sizeof(header_t)) + ALIGN(sizeof(block_t))) / page + 1) * page;              \
    /* println("%s", alloc_size % 4096 == 0 ? "aligned" : "not aligned"); */\
    void *map = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);     \
    if (map == MAP_FAILED) { exit(42); }                                                                \
    if (ptr && ((header_t *)ptr)->full) {                                                               \
        ((header_t *)map)->prev = ptr;                                                                  \
        ((header_t *)ptr)->next = map;                                                                  \
    } else {                                                                                            \
        ((header_t *)map)->prev = nullptr;                                                              \
        ((header_t *)map)->next = nullptr;                                                              \
    }                                                                                                   \
    ptr = map;                                                                                          \
    header_t *header = ptr;                                                                             \
    header->max_blocks = 0;                                                                             \
    header->free_blocks = 0;                                                                            \
    header->offset = ALIGN(sizeof(header_t));                                                           \
    header->block_type = size == TINY ? 0 : size == SMALL ? 1 : 2;                                      \
    header->full = (size == TINY || size == SMALL) ? false : true;                                      \
    header->free = 0;                                                                                   \
    header->chunk_cap = alloc_size;                                                                     \
}                     

#define _MALLOC(size, t, ptr)                                                                           \
    if (!ptr || ((header_t *)ptr)->full) {                                                              \
            init((t == 0 ? TINY : t == 1 ? SMALL : size));                                              \
        }                                                                                               \
        pthread_mutex_lock(&g_mutex);                                                                   \
        header_t *header = ptr;                                                                         \
        block_t *block = (block_t *)((char *)ptr + ALIGN(header->offset));                              \
        if (header->offset == ALIGN(sizeof(header_t))) {                                                \
            block->prev = nullptr;                                                                      \
        }                                                                                               \
        header->max_blocks++;                                                                           \
        block->size = size;                                                                             \
        block->used = 1;                                                                                \
        block->type = t;                                                                                \
        block->free = 0;                                                                                \
        header->offset += size + ALIGN(sizeof(block_t));                                                \
        if (header->offset + ALIGN(sizeof(block_t) + t == 0 ? TINY : t == 1 ? SMALL : 0)                \
            < header->chunk_cap && t != 2) {                                                            \
            ((block_t *)((char *)ptr + header->offset))->prev = block;                                  \
        } else {                                                                                        \
            header->full = 1;                                                                           \
            block->last = 1;                                                                            \
        }                                                                                               \
        pthread_mutex_unlock(&g_mutex);                                                                 \
        return ((void *)((char *)block + ALIGN(sizeof(block_t))))                     

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
    munmap((char *)header, size);                                                                       \
    // memset((char *)header, 0, size);

#define SHOW_ALLOC_MEMORY(small, medium, large)                                                         \
    _SHOW_ALLOC_MEMORY(small, TINY);                                                                    \
    _SHOW_ALLOC_MEMORY(medium, SMALL);                                                                  \
    // _SHOW_ALLOC_MEMORY_LARGE(large)

#define _SHOW_ALLOC_MEMORY(ptr, t) {                                                                    \
    if (!ptr) return;                                                                                   \
    void *list = ptr;                                                                                   \
    while (((header_t *)list)->prev) {                                                                  \
        list = ((header_t *)list)->prev;                                                                \
    }                                                                                                   \
    println("%s : %p", #t, list);                                                                       \
    void *first_chunk = list;                                                                           \
    list = ((char *)list + ALIGN(sizeof(header_t)));                                                    \
    while (42) {                                                                                        \
        if (((block_t *)list)->last) {                                                                  \
            if (((block_t *)list)->used) {                                                              \
                println(                                                                                \
                    "%p - %p : %d bytes",                                                               \
                    (char *)list + sizeof(block_t),                                                     \
                    (char *)list + ((block_t *)list)->size + sizeof(block_t),                           \
                    ((block_t *)list)->size                                                             \
                    );                                                                                  \
            }                                                                                           \
            if (((header_t *)(first_chunk))->next) {                                                    \
                list = ((header_t *)(first_chunk))->next;                                               \
                first_chunk = ((header_t *)(first_chunk))->next;                                        \
                println("%s : %p",#t, list);                                                            \
                list = ((char *)list + ALIGN(sizeof(header_t)));                                        \
            } else {                                                                                    \
                break;                                                                                  \
            }                                                                                           \
        }                                                                                               \
        if(((block_t *)list)->used && !((block_t *)list)->free) {                                       \
            println(                                                                                    \
                "%p - %p : %d bytes",                                                                   \
                (char *)list + sizeof(block_t),                                                         \
                (char *)list + ((block_t *)list)->size + sizeof(block_t),                               \
                ((block_t *)list)->size                                                                 \
                );                                                                                      \
        }                                                                                               \
        list = (char *)list + ALIGN((((block_t *)list)->size + ALIGN(sizeof(block_t))));                \
    }                                                                                                   \
}

#define _SHOW_ALLOC_MEMORY_LARGE(ptr) {                                                                 \
    if (!ptr) return;                                                                                   \
    void *list = ptr;                                                                                   \
    while (((header_t *)list)->prev) {                                                                  \
        list = ((header_t *)list)->prev;                                                                \
    }                                                                                                   \
    println("LARGE : %p\n", list);                                                                      \
    while (list) {                                                                                      \
        println(                                                                                        \
            "%p - %p : %ld bytes\n",                                                                    \
            (char *)list + sizeof(header_t),                                                            \
            (char *)list + (((header_t *)list)->block_size),                                            \
            ((header_t *)list)->block_size                                                              \
            );                                                                                          \
           println("LARGE : %p\n", list);                                                               \
        list = ((header_t *)list)->next;                                                                \
    }                                                                                                   \
}

typedef enum {
    E_TINY,
    E_SMALL,
    E_LARGE
}   E_TYPES;

/* Block header size = 16 */
typedef struct {
   uint8_t                      :3;     /* Paddings bits */
   uint8_t  type                :2;     /* Type of block */
   uint8_t  used                :1;     /* Whether the block is used */
   uint8_t  free                :1;     /* Whether the block is being freed after used */
   uint8_t  last                :1;     /* Whether the block is the last one */
   uint16_t size                  ;     /* Block size */
   void     *prev;                ;     /* Prev block */
} block_t;

/* Chunk header size = 48 */
typedef struct header_s {
    size_t      padding           ;
    uint8_t                     :5;
    uint8_t     block_type      :2;
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
void show_alloc_memory();
// void show_alloc_mem_ex();

void println(const char *fmt, ...);

#endif
