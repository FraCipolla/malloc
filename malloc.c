#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>

t_blocks g_blocks = {0};

static void init()
{
    write(1, "init\n", 5);
    const int page = sysconf(_SC_PAGE_SIZE);
    const int tiny_alloc = (page * ((TINY * 100) / page) + page);
    const int small_alloc = (page * ((SMALL * 100) / page) + page);
    if (!g_blocks.pool.small) {
        g_blocks.pool.small = mmap(0, tiny_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char *small_ptr = g_blocks.pool.small;
        for (int len = 0; len < tiny_alloc; len += TINY) {
            memcpy(
                &small_ptr[len],
                &(t_header){.free=true, .size=TINY, .next=(len == tiny_alloc - TINY) ? nullptr : (void *)&small_ptr[len + TINY]},
                sizeof(t_header)
                );
        }
    }
    if (!g_blocks.pool.medium) {
        g_blocks.pool.medium = mmap(0, small_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        char *medium_ptr = g_blocks.pool.medium;
        for (int len = 0; len < small_alloc; len += SMALL) {
            memcpy(
                &medium_ptr[len],
                &(t_header){.free=true, .size=SMALL, .next=(len == small_alloc - SMALL) ? nullptr : (void *)&medium_ptr[len + SMALL]},
                sizeof(t_header)
                );
        }
    }
    write(1, "end init\n", 10);
}

void *malloc(size_t size)
{
    if (!g_blocks.pool.small || !g_blocks.pool.medium) { init(); }
    if (size < TINY) {
        t_header *current = g_blocks.pool.small;
        current->free = false;
        current->size = TINY;
        g_blocks.pool.small = (char *)g_blocks.pool.small + TINY;
        return current + sizeof(t_header);
    } else if (size < SMALL) {
        t_header *current = g_blocks.pool.medium;
        current->free = false;
        current->size = SMALL;
        g_blocks.pool.medium = (char *)g_blocks.pool.medium + SMALL;
        return current + sizeof(t_header);
    } else {

    }
}

void free(void *ptr)
{
    static void *head;
    static void *tail;
    static int c = 0;
    if (!ptr) {
        return ;
    }
    // printf("count %d\n", c++);
    // ptrdiff_t *diff = ptr - sizeof(t_header);
    t_header *diff = ptr;
    diff -= sizeof(t_header);
    // printf("size %ld\n", diff->size);
    if (!g_blocks.free) {
        // printf("first free\n");
        g_blocks.free = diff;
        head = diff;
        tail = diff;
        ((t_header *)(g_blocks.free))->next = nullptr;
        ((t_header *)(g_blocks.free))->prev = nullptr;
    } else {
        // printf("not first free\n");
        if ((void *)diff > g_blocks.free) {
            while (((t_header *)(g_blocks.free))->next && (t_header *)diff > ((t_header *)(g_blocks.free))) {
                g_blocks.free = ((t_header *)(g_blocks.free))->next;
            }
            if (!((t_header *)(g_blocks.free))->next) {
                ((t_header *)(diff))->next = nullptr;
                ((t_header *)(diff))->prev = ((t_header *)(g_blocks.free));
                ((t_header *)(g_blocks.free))->next = (t_header *)diff;
                tail = diff;
            } else {
                ((t_header *)(diff))->next = ((t_header *)(g_blocks.free));
                ((t_header *)(diff))->prev = ((t_header *)(g_blocks.free))->prev;
                g_blocks.free = diff;
            }
        } else {
            while (((t_header *)(g_blocks.free))->next && (t_header *)diff < ((t_header *)(g_blocks.free))) {
                g_blocks.free = ((t_header *)(g_blocks.free))->prev;
            }
            if (!((t_header *)(g_blocks.free))->prev) {
                ((t_header *)(diff))->prev = nullptr;
                ((t_header *)(diff))->next = ((t_header *)(g_blocks.free));
                ((t_header *)(g_blocks.free))->prev = (t_header *)diff;
                head = diff;
            } else {
                ((t_header *)(diff))->prev = ((t_header *)(g_blocks.free));
                ((t_header *)(diff))->next = ((t_header *)(g_blocks.free))->next;
                g_blocks.free = diff;
            }
        }
    }

    t_header *start = (t_header *)head;
    int count = 0;
    while (start && start->next) {
        // printf("size %ld\n", start->size);
        // printf("ptr %p %p %p\n", start, (char *)start->next, (char *)start + start->size);
        if ((char *)start->next == (char *)start + start->size) {
            printf("adiacent blocks\n");
            count += start->size;
            printf("count: %d\n", count);
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