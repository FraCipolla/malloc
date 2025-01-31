#include "malloc.h"
#include <assert.h>

int main()
{
    /* basic malloc test */
    {
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        *ptr = 42;
        assert(*ptr == 42);
        free(ptr);
    }

    /* basic free test */
    {
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        free(ptr);
    }

    /* double free test */
    {
        int* ptr = (int*) malloc(sizeof(int));
        free(ptr);
        free(ptr);
    }

    /* multiple blocks test */
    {
        int* ptr1 = (int*) malloc(sizeof(int));
        int* ptr2 = (int*) malloc(sizeof(int));
        int* ptr3 = (int*) malloc(sizeof(int));
    
        assert(ptr1 != NULL);
        assert(ptr2 != NULL);
        assert(ptr3 != NULL);
    
        *ptr1 = 1;
        *ptr2 = 2;
        *ptr3 = 3;
    
        assert(*ptr1 == 1);
        assert(*ptr2 == 2);
        assert(*ptr3 == 3);
    
        free(ptr1);
        free(ptr2);
        free(ptr3);
    }

    /* zero bytes malloc */
    {
        void* ptr = malloc(0);
        assert(ptr == NULL || ptr != NULL);
        free(ptr);
    }

    /* realloc with increasing size */
    {
        int* ptr = (int*) malloc(sizeof(int) * 2);
        ptr[0] = 10;
        ptr[1] = 20;
        ptr = (int*) realloc(ptr, sizeof(int) * 4);
        assert(ptr != NULL);  // Verifica che realloc non fallisca
        ptr[2] = 30;
        ptr[3] = 40;

        assert(ptr[0] == 10);
        assert(ptr[1] == 20);
        assert(ptr[2] == 30);
        assert(ptr[3] == 40);

        free(ptr);
    }

    /* realloc with decreasing size */
    {
        int* ptr = (int*) malloc(sizeof(int) * 4);
        ptr[0] = 10;
        ptr[1] = 20;
        ptr[2] = 30;
        ptr[3] = 40;

        ptr = (int*) realloc(ptr, sizeof(int) * 2);
        assert(ptr != NULL);  // Verifica che realloc non fallisca
        assert(ptr[0] == 10);
        assert(ptr[1] == 20);

        free(ptr);
    }

    /* realloc with null pointers */
    {
        int* ptr = (int*) realloc(NULL, sizeof(int) * 4);
        assert(ptr != NULL);
        ptr[0] = 10;
        ptr[1] = 20;
        ptr[2] = 30;
        ptr[3] = 40;

        assert(ptr[0] == 10);
        assert(ptr[1] == 20);
        assert(ptr[2] == 30);
        assert(ptr[3] == 40);

        free(ptr);
    }

    /* realloc with 0 size */
    {
        int* ptr = (int*) malloc(sizeof(int) * 2);
        ptr[0] = 10;
        ptr[1] = 20;

        ptr = (int*) realloc(ptr, 0);
        assert(ptr == NULL);

        free(ptr);
    }

    /* very big malloc */
    {
        size_t large_size = 1024 * 1024 * 1024;
        void* ptr = malloc(large_size);
        assert(ptr != NULL);
        free(ptr);
    }

    /* malloc unitialize memory */
    {
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        free(ptr);
    }

    /* multiple malloc and free in random order */
    {
        int* ptr1 = (int*) malloc(sizeof(int) * 10);
        assert(ptr1 != NULL);
        int* ptr2 = (int*) malloc(sizeof(int) * 20);
        assert(ptr2 != NULL);
        int* ptr3 = (int*) malloc(sizeof(int) * 30);
        assert(ptr3 != NULL);
        int* ptr4 = (int*) malloc(sizeof(int) * 40);
        assert(ptr4 != NULL);
        int* ptr5 = (int*) malloc(sizeof(int) * 50);
        assert(ptr5 != NULL);
        int* ptr6 = (int*) malloc(sizeof(int) * 60);
        assert(ptr6 != NULL);

        ptr1[0] = 1;
        ptr2[0] = 2;
        ptr3[0] = 3;
        ptr4[0] = 4;
        ptr5[0] = 5;
        ptr6[0] = 6;

        free(ptr4);
        free(ptr2);
        free(ptr1);
        free(ptr3);
        free(ptr5);
        free(ptr6);
    }
}