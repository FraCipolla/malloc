#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>
#include <stdlib.h>

chunks g_chunks = {0};

static void init(E_TYPES type)
{
    switch (type)
    {
    case E_TINY: INIT(TINY, g_chunks.small); break;
    case E_SMALL: INIT(SMALL, g_chunks.medium); break;
    default: INIT(LARGE, g_chunks.large); break;
    }
}

void *ft_malloc(size_t size)
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
    size_t offset = size + ALIGN(sizeof(block_t));
    block_t *cast = (block_t *)((char *)ptr - ALIGN(sizeof(block_t)));
    size_t block_size = cast->type == 0 ? TINY : cast->type == 1 ? SMALL : LARGE;
    if (offset > block_size) {
        void *new = ft_malloc(size);
        memcpy(new, ptr, cast->size);
        ft_free(ptr);
        return new;
    }
    return ptr;
}

void ft_free(void *ptr)
{
    if (!ptr) { return ;}

    ptr = (char *)ptr - ALIGN(sizeof(block_t));
    block_t *cast = ptr;
    cast->free = 1;
    size_t size = 0;

    const int block_size = cast->type == E_TINY ? TINY : cast->type == E_SMALL ? SMALL : LARGE;
    header_t *header = (header_t *)((char *)cast - (cast->idx * block_size));
    header->free++;
    if (header->free == header->max_blocks) {
        size = (header->max_blocks + 2) * block_size;
        switch (cast->type)
        {
        case E_TINY: DEALLOC(header, size, g_chunks.small); break;
        case E_SMALL: DEALLOC(header, size, g_chunks.medium); break;
        default: DEALLOC(header, size, g_chunks.large); break;
        }
    }
}

void show_alloc_memory()
{
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
}