#include "malloc.h"
#include <errno.h>

chunks              g_chunks = {0};
pthread_mutex_t		g_mutex = PTHREAD_MUTEX_INITIALIZER;

/* init memory zones */
static inline void* init(size_t size, E_TYPES type)
{
    switch (type)
    {
    case E_TINY: INIT(E_TINY, TINY, g_chunks.small);
    case E_SMALL: INIT(E_SMALL, SMALL, g_chunks.medium);
    default: INIT(E_LARGE, size, g_chunks.large);
    }
    return nullptr;
}

/* allocate (2 * sizeof(size_t)) aligned memory block */
void *malloc(size_t size)
{
    pthread_mutex_lock(&g_mutex);
    void *return_ptr = nullptr;
    if (size <= (TINY - BLOCK_ALIGN())) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - BLOCK_ALIGN())) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {
        _MALLOC(size, E_LARGE, g_chunks.large);
    }
    pthread_mutex_unlock(&g_mutex);
    return return_ptr;
}

void *calloc(size_t nmemb, size_t size)
{
    pthread_mutex_lock(&g_mutex);
    void *return_ptr = nullptr;
    size_t el_size = size;
    size = size * nmemb;
    if (size <= (TINY - BLOCK_ALIGN())) {
        _MALLOC(size, E_TINY, g_chunks.small);
    } else if (size <= (SMALL - BLOCK_ALIGN())) {
        _MALLOC(size, E_SMALL, g_chunks.medium);
    } else {
        _MALLOC(size, E_LARGE, g_chunks.large);
    }
    
    switch (el_size)
    {
    case 2:
        uint16_t *p16 = return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p16[i] = 0;
        }
        break;
    case 4:
        uint32_t *p32 = (uint32_t *)return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p32[i] = 0;
        }
        break;
    case 8:
        uint64_t *p64 = return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p64[i] = 0;
        }
        break;
    default:
        uint8_t *p = return_ptr;
        for (size_t i = 0; i < nmemb; i++) {
            p[i] = 0;
        }
        break;
    }
    // sizeof(uint64_t) == 8
    pthread_mutex_unlock(&g_mutex);
    return return_ptr;
}

/*
* The  reallocarray()  function changes the size of the memory block pointed to by ptr to be large enough for an
* array of nmemb elements, each of which is size bytes.  It is equivalent to the call
* 
*   realloc(ptr, nmemb * size);
* 
* However, unlike that realloc() call, reallocarray() fails safely in the case where  the  multiplication  would
* overflow.  If such an overflow occurs, reallocarray() returns NULL, sets errno to ENOMEM, and leaves the orig‐
* inal block of memory unchanged.
*/
void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
    long long x = nmemb * size;
    if ((nmemb != 0 && size != 0) && nmemb != x / size) {
        // overflow handling
        errno = ENOMEM;
        return nullptr;
    }
    return realloc(ptr, nmemb * size);
}

/* 
 * realloc passed pointer. If no block after the passed one exists
 * or if the block extra size is enought, set the new size and return the pointer.
 * Else, call malloc for a new pointer, copy the old pointer to the new one, and free the old pointer
*/
void *realloc(void *ptr, size_t size)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Attempt to realloc a null pointer: %p\n", ptr);
        pthread_mutex_unlock(&g_mutex);
        return nullptr;
    } else if (size == 0) {
        g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Reallocing pointer giving 0 size (equale free): %p\n", ptr);
        _FREE(ptr)
        ptr = nullptr;
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    }

    g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Reallocing pointer: %p for size: %d\n", ptr, size);
    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());
    if (size <= cast->size || cast->extra_size + cast->size >= size) {
        g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "   Enought size in block %p, no need to realloc\n", ptr);
        cast->extra_size = cast->size + cast->extra_size - size; 
        cast->size = size;
    } else if (!cast->next && cast->type != 2) {
        g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "   Pointer %p is the last allocated block of the chunk, no need to realloc\n", ptr);
        cast->size = size;
        cast->extra_size = ALIGN(size);
    } else {
        g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "   Not enought size (%d) for pointer %p, allocate a new pointer and free the old one\n", size, ptr);
        void *return_ptr = nullptr;
        if (size <= (TINY - BLOCK_ALIGN())) {
            _MALLOC(size, E_TINY, g_chunks.small);
        } else if (size <= (SMALL - BLOCK_ALIGN())) {
            _MALLOC(size, E_SMALL, g_chunks.medium);
        } else {
            _MALLOC(size, E_LARGE, g_chunks.large);
        }
        ft_memcpy(return_ptr, (void *)cast + BLOCK_ALIGN(), cast->size);
        _FREE(ptr)
        return return_ptr;
    }
    pthread_mutex_unlock(&g_mutex);
    return ptr;
}

/*
 * free the passed pointer. If it's the last pointer, delete it and move 1 block back.
 * If the block has adjacent free blocks, merge them to avoid fragmentation.
 */
void free(void *ptr)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        g_chunks.history.idx += add_to_history(&g_chunks.history.buffer[g_chunks.history.idx], \
            "Attempt to free a null pointer: %p\n", ptr);
        pthread_mutex_unlock(&g_mutex);
        return ;
    }

    _FREE(ptr)
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_mem()
{
    pthread_mutex_lock(&g_mutex);
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_mem_ex()
{
    pthread_mutex_lock(&g_mutex);
    print("%s", g_chunks.history.buffer);
    pthread_mutex_unlock(&g_mutex);
}

void hex_dump() {
    pthread_mutex_lock(&g_mutex);
    HEX_DUMP(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

void print_memory() {
    pthread_mutex_lock(&g_mutex);
    PRINT_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}
