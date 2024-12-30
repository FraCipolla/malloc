#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#if __STDC_VERSION__ != 202311L
#define false 0
#define true !false
#define nullptr NULL
#include <stdbool.h>
#endif

#define DEBUG(x) write(1, x, strlen(x))

#define MAP_ANONYMOUS 0x20  /* Don't use a file. */
#define ALIGNMENT sizeof(block_t) // must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

#define TINY (1 << 8)
#define SMALL (1 << 10)
#define LARGE (1 << 12)

#define TYPE_TO_SIZE(T) (T == 0 ? TINY : T == 1 ? SMALL : LARGE)
#define PERROR(status, msg) perror(msg); exit(status)
#define INIT(size, ptr)                                                                                  \
{                                                                                                        \
    const int page = getpagesize();                                                                      \
    const int alloc_size = (size * 100) + page;                                                          \
    void *map = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);      \
    if (map == MAP_FAILED) { PERROR(42, "mmap failed"); }                                                \
    if (ptr && ((header_t *)ptr)->full) {                                                                \
        ((header_t *)map)->prev = ptr;                                                                   \
        ((header_t *)ptr)->next = map;                                                                   \
    } else {                                                                                             \
        ((header_t *)map)->prev = nullptr;                                                               \
        ((header_t *)map)->next = nullptr;                                                               \
    }                                                                                                    \
    ptr = map;                                                                                           \
    printf("ptr %p\n", ptr);                                                                             \
    header_t *header = ptr;                                                                              \
    header->free = 0;                                                                                    \
    header->block_type = 0;                                                                              \
    header->block_index = 1;                                                                             \
    header->max_blocks = (alloc_size / size) - 2;                                                        \
    header->full = false;                                                                                \
    ((block_t *)(((char *)ptr) + size))->first = 1;                                                      \
    ((block_t *)(((char *)ptr) + (alloc_size - (size * 2))))->last = 1;                                  \
    ((block_t *)(((char *)ptr) + (alloc_size - (size * 2))))->idx = header->max_blocks;                  \
}                     

#define _MALLOC(size, t, ptr)                                                                           \
    if (!ptr || ((header_t *)ptr)->full) {                                                              \
            init(t);                                                                                    \
        }                                                                                               \
        header_t *header = ptr;                                                                         \
        block_t *block = ptr + (header->block_index * TYPE_TO_SIZE(t));                                 \
        block->size = size;                                                                             \
        block->used = 1;                                                                                \
        block->type = t;                                                                                \
        block->free = 0;                                                                                \
        block->idx = header->block_index;                                                               \
        if (block->idx == header->max_blocks) {                                                         \
            header->full = true;                                                                        \
        } else {                                                                                        \
            header->block_index++;                                                                      \
        }                                                                                               \
        return ((char *)block + ALIGN(sizeof(block_t)))                     

typedef enum {
    E_TINY,
    E_SMALL,
    E_LARGE
}   E_TYPES;

typedef struct {
   size_t               :2;     /* Paddings bits */
   size_t type          :2;     /* Type of block: tiny(0), small(1) or large(2) */
   size_t first         :1;     /* Marks the first block */
   size_t last          :1;     /* Marks the last block */
   size_t used          :1;     /* Whether the block is used */
   size_t free          :1;     /* Whether the block is being freed after used */
   size_t idx           :8;     /* Block index, for debugging purpose */
   size_t size          ;       /* Block size in bytes. Max 2^32 or 2^64 depending on system architecture */
} block_t;

/* Chunk header */
typedef struct header_s {
    int     block_type;
    int     block_index;
    int     max_blocks;
    int     free;
    bool    full;
    void    *next;
    void    *prev;
}   header_t;

/* Chunk footer */
typedef struct footer_s {
    void *next;
}   footer_t;

typedef struct s_chunks {
    void            *small;
    void            *medium;
    void            *large;
}   chunks;

void ft_free(void *ptr);
void *ft_malloc(size_t size);
void *realloc(void *ptr, size_t size);
void show_alloc_memory();
void show_alloc_mem_ex();

#endif
