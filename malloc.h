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
#define TTINY 0
#define TSMALL 1
#define TLARGE 2

typedef struct {
   size_t               :2;     /* Paddings bits */
   size_t type          :2;     /* Type of block: tiny(1), small(2) or large(3) */
   size_t first         :1;     /* Marks the first block */
   size_t last          :1;     /* Marks the last block */
   size_t used          :1;     /* Whether the block is used */
   size_t free          :1;     /* Whether the block is being freed after used */
   size_t idx           :8;     /* Block index, for debugging purpose */
   size_t size          ;       /* Block size in bytes. Max 2^32 or 2^64 depending on system architecture */
} block_t;


/* Only for last block */
typedef struct footer_s {
    void *next;
}   footer_t;

typedef struct s_chunks {
    void            *small;
    void            *medium;
    void            *large;
    void            *alloc_list_s;
    void            *alloc_list_m;
    void            *alloc_list_l;
}   chunks;

void ft_free(void *ptr);
void *ft_malloc(size_t size);
void *realloc(void *ptr, size_t size);
void show_alloc_memory();
void show_alloc_mem_ex();

#endif
