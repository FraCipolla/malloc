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
#define ALIGNMENT 8 // must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define TINY (1 << 8)
#define SMALL (1 << 10)
#define LARGE (1 << 12)

typedef enum {
    ETINY,
    ESMALL,
    ELARGE
}   type;

typedef struct s_header {
    bool            free;
    size_t          size;
    struct s_header *next;
    struct s_header *prev;
}   t_header;

typedef struct s_pool {
    void *small;
    void *medium;
    void *large;
}   pool;

typedef struct s_blocks {
    pool    pool;
    void    *free;
}   t_blocks;

extern t_blocks g_blocks;

void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void show_alloc_memory();

#endif