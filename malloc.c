#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>
#include <stdlib.h>

t_blocks g_blocks = {0};

static void init()
{
    DEBUG("init\n");
    const int page = sysconf(_SC_PAGE_SIZE);
    const int tiny_alloc = (TINY * 100) + page;
    printf("Tiny alloc: %d\n", tiny_alloc);
    const int small_alloc = (page * ((SMALL * 100) / page) + page);
    if (!g_blocks.small) {
        g_blocks.small = mmap(NULL, tiny_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (g_blocks.small == MAP_FAILED) {
            perror("mmap failed");
            exit(42);
        }
    }
        block_t *first = g_blocks.small;
        first->first = 1;
        first->type = 1;
        first->idx = 0;
        block_t *last = (block_t *)(((char *)g_blocks.small) + (tiny_alloc - TINY));
        last->last = 1;
        last->type = 1;
        last->size = TINY - ALIGN(sizeof(block_t));
    if (!g_blocks.medium) {
    }
}

void *ft_malloc(size_t size)
{
    // DEBUG("malloc\n");
    if (size <= (TINY - ALIGN(sizeof(block_t)))) {
        if (!g_blocks.small) { init(); }
        block_t *cast = g_blocks.small;
        cast->size = TINY - ALIGN(sizeof(block_t));
        cast->used = 1;
        cast->type = 1;
        cast->free = 0;
        g_blocks.small += TINY;
        if (cast->last) {
            printf("cast->last\n");
            g_blocks.small = nullptr;
        } else {
            ((block_t *)(g_blocks.small))->idx = cast->idx + 1;
        }
        return ((char *)cast + ALIGN(sizeof(block_t)));
    } else if (size <= (SMALL - ALIGN(sizeof(block_t)))) {
        if (!g_blocks.small || !g_blocks.medium) { init(); }
    } else {

    }
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) {
        return nullptr;
    }
    size_t offset = size + ALIGN(sizeof(block_t));
    block_t    *cast = ptr;
    cast -= ALIGN(sizeof(block_t));
    if (offset > cast->size) {
        void *new = malloc(size);
        memcpy(new, ptr, cast->size);
        free(ptr);
        return new;
    }
    return ptr;
}

void ft_free(void *ptr)
{
    // DEBUG("free\n");
    if (!ptr) { return ;}

    ptr = (char *)ptr - ALIGN(sizeof(block_t));
    block_t *cast = ptr;
    cast->free = 1;
    size_t size = 0;
    bool dealloc = false;
    int type = 0;
    void *start = nullptr;

    if (cast->first) {
        start = cast;
        while (cast && !cast->last && cast->used && cast->free) {
            size += cast->size + ALIGN(sizeof(block_t));
            cast = (block_t *)((char *)cast + cast->size + ALIGN(sizeof(block_t)));
        }
        if (cast->last && cast->used && cast->free) {
            size += cast->size + ALIGN(sizeof(block_t));
            type = cast->type;
            dealloc = true;
        }
    } else if (cast->last) {
        printf("last found!\n");
        while (cast && !cast->first && cast->used && cast->free) {
            size += cast->size + ALIGN(sizeof(block_t));
            cast = (block_t *)((char *)cast - (cast->size + ALIGN(sizeof(block_t))));
        }
        if (cast->first && cast->used && cast->free) {
            size += cast->size + ALIGN(sizeof(block_t));
            type = cast->type;
            start = cast;
            dealloc = true;
        }
    } else {
        block_t *tmp = cast;
        // printf("used %d && free %d\n", cast->used, cast->free);
        while (cast && !cast->first && cast->used && cast->free) {
            size += cast->size + ALIGN(sizeof(block_t));
            cast = (block_t *)((char *)cast - (cast->size + ALIGN(sizeof(block_t))));
        }
        if (cast->first) {
            start = cast;
            cast = tmp;
            while (cast && !cast->last && cast->used && cast->free) {
                size += cast->size + ALIGN(sizeof(block_t));
                cast = (block_t *)((char *)cast + (cast->size + ALIGN(sizeof(block_t))));
            }
            if (cast->last && cast->used && cast->free) {
                size += cast->size + ALIGN(sizeof(block_t));
                type = cast->type;
                dealloc = true;
            }
        }
    }
    if (dealloc) {
        printf("munmap! size: %ld\n", size);
        munmap(start, size);
        switch (type)
        {
        case TTINY:
            printf("case TTINY\n");
            g_blocks.small = nullptr;
            break;
        case TSMALL:
            g_blocks.medium = nullptr;
            break;
        case TLARGE:
            g_blocks.large = nullptr;
            break;
        default:
            break;
        }
    }
}

void show_alloc_memory()
{

}