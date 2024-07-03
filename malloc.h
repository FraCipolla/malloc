#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#define ALIGNMENT 8 // must be a power of 2
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

typedef struct s_block {
    size_t size;
    void *addr;
    struct s_block *next;
}   m_block;

void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

#endif