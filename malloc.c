#include "malloc.h"

#include <stdio.h>
#include <bits/mman-linux.h>
#include <string.h>

t_blocks g_blocks;
// void free(void *ptr) {
//     if (!ptr) { return; }
// }

static void init() {
    const int page = sysconf(_SC_PAGE_SIZE);
    const int tiny_alloc = (page * ((TINY * 100) / page) + page);
    const int small_alloc = (page * ((SMALL * 100) / page) + page);
    g_blocks = (t_blocks){
        .small=mmap(0, tiny_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
        .medium=mmap(0, small_alloc, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
        // .large=mmap(0, page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
        };
    char *small_ptr = g_blocks.small;
    int index = 0;
    for (int len = 0; len < tiny_alloc; len += TINY) {
        if (len == tiny_alloc - TINY) {
            memcpy(&small_ptr[len], &(t_header){.free=true, .size=TINY, .next=nullptr, .index=index++}, sizeof(t_header));
            break;
        }
        memcpy(&small_ptr[len], &(t_header){.free=true, .size=TINY, .next=(void *)&small_ptr[len + TINY], .index=index++}, sizeof(t_header));
    }
    char *medium_ptr = g_blocks.medium;
    for (int len = 0; len < small_alloc; len += SMALL) {
        if (len == small_alloc - SMALL) {
            memcpy(&medium_ptr[len], &(t_header){.free=true, .size=SMALL, .next=nullptr, .index=index++}, sizeof(t_header));
            break;
        }
        memcpy(&medium_ptr[len], &(t_header){.free=true, .size=SMALL, .next=(void *)&medium_ptr[len + SMALL], .index=index++}, sizeof(t_header));
    }
}

void *ft_malloc(size_t size) {
    if (!g_blocks.small || !g_blocks.medium) { init(); }
    if (size < TINY) {
        printf("size tiny\n");
        t_header *current = g_blocks.small;
        while (current->next) {
            if (current->free) {
                current->free = false;
                return current + sizeof(t_header);
            }
            current = current->next;
        }
    } else if (size < SMALL) {

    } else {

    }
}