#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <sys/mman.h>

#define ALIGNMENT 8 // must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define TINY (1 << 8)
#define SMALL (1 << 10)
#define LARGE (1 << 12)

// typedef struct s_block {
//     int free: 1;
//     size_t size;
//     void *addr;
//     struct s_block *next;
// }   m_block;

/*
    small block:
    header:
    1bit -> free/not free;
    8byte -> size_t;
    8byte -> next_ptr;
    header size = 64 + 64 + 129;
*/

typedef struct s_block {
    void *small;
    void *medium;
    void *large;
    void *to_free;
}   t_blocks;

extern t_blocks g_blocks;

void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void show_alloc_memory();

#endif