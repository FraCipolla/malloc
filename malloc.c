#include "malloc.h"

t_blocks g_blocks;

void free(void *ptr) {
    if (!ptr) { return; }
}

void *malloc(size_t size) {
    if (size < TINY) {
        void *current = g_blocks.small;
        if (!current) {
            current = mmap(
                0,
                getpagesize(),
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE,
                0,
                0
            );
        }
        while (current) {
            char MSB = (size_t)current & 0xFF00000000000000;
        }
    } else if (size < SMALL) {

    } else {

    }
}