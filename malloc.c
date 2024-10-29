#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>

t_blocks g_blocks;

static void init()
{
    const int page = sysconf(_SC_PAGE_SIZE);
    const int tiny_alloc = (page * ((TINY * 100) / page) + page);
    const int small_alloc = (page * ((SMALL * 100) / page) + page);
    if (!g_blocks.small) {
        g_blocks.small = mmap(0, tiny_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char *small_ptr = g_blocks.small;
        for (int len = 0; len < tiny_alloc; len += TINY) {
            memcpy(
                &small_ptr[len],
                &(t_header){.free=true, .size=TINY, .next=(len == tiny_alloc - TINY) ? nullptr : (void *)&small_ptr[len + TINY]},
                sizeof(t_header)
                );
        }
    }
    if (!g_blocks.medium) {
        g_blocks.medium = mmap(0, small_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char *medium_ptr = g_blocks.medium;
        for (int len = 0; len < small_alloc; len += SMALL) {
            memcpy(
                &medium_ptr[len],
                &(t_header){.free=true, .size=SMALL, .next=(len == small_alloc - SMALL) ? nullptr : (void *)&medium_ptr[len + SMALL]},
                sizeof(t_header)
                );
        }
    }
}

void *malloc(size_t size)
{
    if (!g_blocks.small || !g_blocks.medium) { init(); }
    if (size < TINY) {
        t_header *current = g_blocks.small;
        current->free = false;
        current->size = size;
        g_blocks.small = (char *)g_blocks.small + TINY;
        return current + sizeof(t_header);
    } else if (size < SMALL) {
        t_header *current = g_blocks.medium;
        current->free = false;
        current->size = size;
        g_blocks.medium = (char *)g_blocks.medium + SMALL;
        return current + sizeof(t_header);
    } else {

    }
}

void free(void *ptr)
{
    if (!ptr) {
        return ;
    }
    t_header *cast = ptr;
    cast -= sizeof(t_header);
    cast->free = true;
    linked_list list = cast->size < TINY ? g_blocks.free.small : cast->size < SMALL ? g_blocks.free.medium : g_blocks.free.large;
    t_header *free_blocks = (t_header *)list.node;
    if (!free_blocks) {
        free_blocks = cast;
        list.head = cast;
        list.tail = cast;
        free_blocks->next = nullptr;
        free_blocks->prev = nullptr;
    } else {
        if (cast > free_blocks) {
            while (free_blocks->next && cast > free_blocks) {
                free_blocks = free_blocks->next;
            }
            if (!free_blocks->next) {
                free_blocks->next = cast;
                list.tail = cast;
                cast->next = nullptr;
                cast->prev = free_blocks;
            } else {
                cast->next = free_blocks;
                cast->prev = free_blocks->prev;
                free_blocks = cast;
            }
        } else {
            while (free_blocks->prev && cast < free_blocks) {
                free_blocks = free_blocks->prev;
            }
            if (!free_blocks->prev) {
                list.head = cast;
                free_blocks->prev = cast;
                cast->prev = nullptr;
                cast->next = free_blocks;
            } else {
                cast->prev = free_blocks;
                cast->next = free_blocks->next;
                free_blocks = cast;
            }
        }
    }

    int offset = cast->size < TINY ? TINY : cast->size < SMALL ? SMALL : LARGE;
    t_header *start = (t_header *)list.head;
    int count = 0;
    while (start && start->next) {
        printf("count: %d\n", count);
        if (start->next == start + offset) {
            count += offset;
            if (count == sysconf(_SC_PAGE_SIZE)) {
                printf("free blocks\n");
                munmap(start - count, count);
            }
        } else {
            count = 0;
        }
        start = start->next;
    }
}