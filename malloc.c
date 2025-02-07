#include "malloc.h"
#include <errno.h>
#include <fcntl.h>

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

/*
 * The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory.  The memory is not initialized.  If size
 * is 0, then malloc() returns a unique pointer value that can later
 * be successfully passed to free().  (See "Nonportable behavior" for
 * portability issues.)
*/
void *malloc(size_t size)
{
    // print("size_t %d\n", sizeof(size_t));
    // print("ptr %d\n", sizeof(void *));
    // print("block %d\n", sizeof(block_t));
    // print("header %d\n", sizeof(header_t));
    // print("block align %d\n", BLOCK_ALIGN());
    // print("header align %d\n", HEADER_ALIGN());
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

/* he calloc() function allocates memory for an array of n elements
 * of size bytes each and returns a pointer to the allocated memory.
 * The memory is set to zero.  If n or size is 0, then calloc()
 * returns a unique pointer value that can later be successfully
 * passed to free().
 *
 * If the multiplication of n and size would result in integer
 * overflow, then calloc() returns an error.  By contrast, an integer
 * overflow would not be detected in the following call to malloc(),
 * with the result that an incorrectly sized block of memory would be
 * allocated:
 *
 *  malloc(n * size);
 */
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
 * The realloc() function changes the size of the memory block
 * pointed to by ptr to size bytes.  The contents of the memory will
 * be unchanged in the range from the start of the region up to the
 * minimum of the old and new sizes.  If the new size is larger than
 * the old size, the added memory will not be initialized.

 * If ptr is NULL, then the call is equivalent to malloc(size), for
 * all values of size.

 * If size is equal to zero, and ptr is not NULL, then the call is
 * equivalent to free(ptr) (but see "Nonportable behavior" for
 * portability issues).

 * Unless ptr is NULL, it must have been returned by an earlier call
 * to malloc or related functions.  If the area pointed to was moved,
 * a free(ptr) is done.
*/
void *realloc(void *ptr, size_t size)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        pthread_mutex_unlock(&g_mutex);
        return malloc(size);
    } else if (size == 0) {
        add_to_history("Reallocing pointer giving 0 size (equale free): %p\n", ptr);
        _FREE(ptr)
        ptr = nullptr;
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    }

    add_to_history("Reallocing pointer: %p for size: %d\n", ptr, size);
    block_t *cast = (block_t *)((char *)ptr - BLOCK_ALIGN());
    if (size <= cast->size || cast->extra_size + cast->size >= size) {
        add_to_history("   Enought size in block %p, no need to realloc\n", ptr);
        cast->extra_size = cast->size + cast->extra_size - size; 
        cast->size = size;
    } else if (!cast->next && cast->type != 2) {
        add_to_history("   Pointer %p is the last allocated block of the chunk, no need to realloc\n", ptr);
        cast->size = size;
        cast->extra_size = ALIGN(size);
    } else {
        add_to_history("   Not enought size (%d) for pointer %p, allocate a new pointer and free the old one\n", size, ptr);
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
 * The free() function frees the memory space pointed to by ptr,
 * which must have been returned by a previous call to malloc() or
 * related functions.  Otherwise, or if ptr has already been freed,
 * undefined behavior occurs.  If ptr is NULL, no operation is
 * performed.
*/
void free(void *ptr)
{
    pthread_mutex_lock(&g_mutex);
    if (!ptr) {
        add_to_history("Attempt to free a null pointer: %p\n", ptr);
        pthread_mutex_unlock(&g_mutex);
        return ;
    }
    
    if (((block_t *)((char *)ptr - BLOCK_ALIGN()))->used) {
        _FREE(ptr)
    }
    pthread_mutex_unlock(&g_mutex);
}

/* Print allocated memory in term of size and address range */
void show_alloc_mem()
{
    pthread_mutex_lock(&g_mutex);
    SHOW_ALLOC_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

/* Print an history of allocations and deallocations */
void show_alloc_mem_ex()
{
    pthread_mutex_lock(&g_mutex);
    int fd = open("malloc_history.txt", O_RDONLY, 0644);
    char buff[1024] = {0};
    int bytes_read = 0;
    if (fd != -1) {
        while ((bytes_read = read(fd, buff, 1024))) {
            write(1, buff, bytes_read);
        }
        close(fd);
    }
    pthread_mutex_unlock(&g_mutex);
}

/*
 * Print an hexdump of each allocated block.
 * Display the starting address and the following 32 bytes in hexadecimal format,
 * grouped 2 at a time.
*/
void hex_dump() {
    pthread_mutex_lock(&g_mutex);
    HEX_DUMP(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}

/*
 * Print the allocated memory showing the chunk header,
 * the block header, the used memory and the free memory 
*/
void print_memory() {
    pthread_mutex_lock(&g_mutex);
    PRINT_MEMORY(g_chunks.small, g_chunks.medium, g_chunks.large);
    pthread_mutex_unlock(&g_mutex);
}
