#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>

t_blocks g_blocks = {0};

static void init()
{
    DEBUG("init\n");
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
}

void *malloc(size_t size)
{
    if (!g_blocks.pool.small || !g_blocks.pool.medium) { init(); }
    if (size < (TINY - ALIGN(sizeof(t_header)))) {
        t_header *current = g_blocks.pool.small;
        current->free = false;
        current->size = TINY;
        if (!current->next) {
            g_blocks.pool.small = nullptr;
        } else {
            g_blocks.pool.small = (char *)g_blocks.pool.small + TINY;
        }
        return current + ALIGN(sizeof(t_header));
    } else if (size < (SMALL - ALIGN(sizeof(t_header)))) {
        t_header *current = g_blocks.pool.medium;
        current->free = false;
        current->size = SMALL;
        g_blocks.pool.medium = (char *)g_blocks.pool.medium + SMALL;
        return current + ALIGN(sizeof(t_header));
    } else {

    }
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) {
        return nullptr;
    }
    size_t offset = size + ALIGN(sizeof(t_header));
    t_header    *cast = ptr;
    cast -= ALIGN(sizeof(t_header));
    if (offset > cast->size) {
        void *new = malloc(size);
        memcpy(new, ptr, cast->size);
        free(ptr);
        return new;
    }
    return ptr;
}

void free(void *ptr)
{
    static void *head;
    static void *tail;
    
    if (!ptr) {
        return ;
    }
    t_header *diff = ptr;
    void *tmp = g_blocks.free;
    diff -= ALIGN(sizeof(t_header));
    if (!tmp) {
        tmp = diff;
        head = diff;
        tail = diff;
        ((t_header *)(tmp))->next = nullptr;
        ((t_header *)(tmp))->prev = nullptr;
    } else {
        if ((void *)diff > tmp) {
            while (((t_header *)(tmp))->next && (t_header *)diff > ((t_header *)(tmp))) {
                tmp = ((t_header *)(tmp))->next;
            }
            if (!((t_header *)(tmp))->next) {
                ((t_header *)(diff))->next = nullptr;
                ((t_header *)(diff))->prev = ((t_header *)(tmp));
                ((t_header *)(tmp))->next = (t_header *)diff;
                tail = diff;
            } else {
                ((t_header *)(diff))->next = ((t_header *)(tmp));
                ((t_header *)(diff))->prev = ((t_header *)(tmp))->prev;
                tmp = diff;
            }
        } else {
            while (((t_header *)(tmp))->next && (t_header *)diff < ((t_header *)(tmp))) {
                tmp = ((t_header *)(tmp))->prev;
            }
            if (!((t_header *)(tmp))->prev) {
                ((t_header *)(diff))->prev = nullptr;
                ((t_header *)(diff))->next = ((t_header *)(tmp));
                ((t_header *)(tmp))->prev = (t_header *)diff;
                head = diff;
            } else {
                ((t_header *)(diff))->prev = ((t_header *)(tmp));
                ((t_header *)(diff))->next = ((t_header *)(tmp))->next;
                tmp = diff;
            }
        }
    }

    t_header *start = (t_header *)head;
    t_header *prev = nullptr;
    int count = 0;
    while (start && start->next) {
        if ((char *)start->next == (char *)start + start->size) {
            count += start->size;
            if (count % sysconf(_SC_PAGE_SIZE) == 0) {
                start = start->next;
                if (!prev) {
                    start->prev = nullptr;
                    head = start;
                } else {
                    prev = start;
                }
                munmap(start - count, count);
            }
        } else {
            prev = start;
            count = 0;
        }
        start = start->next;
    }
}

void show_alloc_memory()
{

}