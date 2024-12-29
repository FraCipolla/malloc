#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>
#include <stdlib.h>

chunks g_chunks = {0};
size_t sizes[] = {TINY, SMALL, LARGE};

static void init()
{
    DEBUG("init\n");
    const int page = sysconf(_SC_PAGE_SIZE);
    const int tiny_alloc = (TINY * 100) + page;
    printf("Tiny alloc: %d\n", tiny_alloc);
    const int small_alloc = (page * ((SMALL * 100) / page) + page);
    if (!g_chunks.small) {
        g_chunks.small = mmap(NULL, tiny_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (g_chunks.small == MAP_FAILED) {
            perror("mmap failed");
            exit(42);
        }
    }
        block_t *first = g_chunks.small;
        first->first = 1;
        first->idx = 0;
        ((block_t *)(((char *)g_chunks.small) + (tiny_alloc - TINY)))->last = 1;
    if (!g_chunks.medium) {
    }
}

void *ft_malloc(size_t size)
{
    // DEBUG("malloc\n");
    if (size <= (TINY - ALIGN(sizeof(block_t)))) {
        if (!g_chunks.small) {
            init();
            g_chunks.alloc_list_s = g_chunks.small;
        }
        block_t *cast = g_chunks.small;
        cast->size = size;
        cast->used = 1;
        cast->type = TTINY;
        cast->free = 0;
        g_chunks.small += TINY;
        if (cast->last) {
            printf("last chunk\n");
            g_chunks.small = nullptr;
            init();
            ((footer_t *)((char *)cast + TINY) - sizeof(footer_t))->next = g_chunks.small;
            ft_malloc(size);
        } else {
            ((block_t *)(g_chunks.small))->idx = cast->idx + 1;
        }
        return ((char *)cast + ALIGN(sizeof(block_t)));
    } else if (size <= (SMALL - ALIGN(sizeof(block_t)))) {
        if (!g_chunks.small || !g_chunks.medium) { init(); }
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
    if (offset > sizes[cast->type]) {
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
            g_chunks.small = nullptr;
            break;
        case TSMALL:
            g_chunks.medium = nullptr;
            break;
        case TLARGE:
            g_chunks.large = nullptr;
            break;
        default:
            break;
        }
    }
}

void show_alloc_memory()
{
    void *small_list = g_chunks.alloc_list_s;
    printf("TINY : %p\n", small_list);
    while (42) {
        if (((block_t *)small_list)->last) {
            if (((footer_t *)((char *)small_list + TINY) - sizeof(footer_t))->next) {
                small_list = ((footer_t *)((char *)small_list + TINY) - sizeof(footer_t))->next;
                printf("TINY : %p\n", small_list);
            } else {
                break;
            }
        }
        if(((block_t *)small_list)->used && !((block_t *)small_list)->free) {
            printf("%p - %p : %ld bytes\n", (char *)small_list + sizeof(block_t), (char *)small_list + ((block_t *)small_list)->size + sizeof(block_t), ((block_t *)small_list)->size);
        }
        small_list = ((char *)small_list) + TINY;
    }
}