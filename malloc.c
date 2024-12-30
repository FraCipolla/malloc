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
    case E_TINY:
        INIT(TINY, g_chunks.small);
        break;
    case E_SMALL:
        INIT(SMALL, g_chunks.medium);
        break;
    default:
        break;
    }
}

void *ft_malloc(size_t size)
{
    // DEBUG("malloc\n");
    if (size <= (TINY - ALIGN(sizeof(block_t)))) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - ALIGN(sizeof(block_t)))) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {

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
    bool dealloc = false;
    void *start = nullptr;

    const int block_size = cast->type == E_TINY ? TINY : cast->type == E_SMALL ? SMALL : LARGE;
    header_t *header = (header_t *)((char *)cast - (cast->idx * block_size));
    header->free++;
    if (header->free == header->max_blocks) {
        dealloc = true;
        size = (header->max_blocks + 2) * block_size;
        start = header;
    }
    if (dealloc) {
        printf("munmap! size: %ld\n", size);
        switch (cast->type)
        {
        case E_TINY:
            printf("case E_TINY\n");
            printf("size %ld\n", size);
            if (header->prev && header->next) {
                ((header_t *)(header->prev))->next = header->next;
                // merge prev with next
                g_chunks.small = nullptr;
            } else if (header->next) {
                g_chunks.small = header->next;
            } else if (header->prev) {
                ((header_t *)(header->prev))->next = nullptr;
            } else {
                g_chunks.small = nullptr;
            }
            munmap(start, size);
            break;
        case E_SMALL:
            g_chunks.medium = nullptr;
            break;
        case E_LARGE:
            g_chunks.large = nullptr;
            break;
        default:
            break;
        }
    }
}

void show_alloc_memory()
{
    void *small_list = g_chunks.small;
    printf("TINY : %p\n", small_list);
    while (((header_t *)small_list)->prev) {
        small_list = ((header_t *)small_list)->prev;
    }
    void *first_chunk = small_list;
    small_list = (char *)small_list + TINY;
    while ((((char *)small_list) + TINY)) {
        if (((block_t *)small_list)->last) {
            if (((block_t *)small_list)->used) {
                printf(
                    "%p - %p : %ld bytes\n",
                    (char *)small_list + sizeof(block_t),
                    (char *)small_list + ((block_t *)small_list)->size + sizeof(block_t),
                    ((block_t *)small_list)->size
                    );
            }
            if (((header_t *)(first_chunk))->next) {
                small_list = ((header_t *)(first_chunk))->next;
                first_chunk = ((header_t *)(first_chunk))->next;
                small_list = (char *)small_list + TINY;
                printf("TINY : %p\n", small_list);
            } else {
                break;
            }
        }
        if(((block_t *)small_list)->used && !((block_t *)small_list)->free) {
            printf(
                "%p - %p : %ld bytes\n",
                (char *)small_list + sizeof(block_t),
                (char *)small_list + ((block_t *)small_list)->size + sizeof(block_t),
                ((block_t *)small_list)->size
                );
        }
        small_list = ((char *)small_list) + TINY;
    }
}