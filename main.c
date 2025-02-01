#include "malloc.h"
#include <assert.h>

int main()
{
    /* basic malloc test */
    {
        print("\nbasic malloc test\n\n");
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        *ptr = 42;
        assert(*ptr == 42);
        free(ptr);
    }

    /* basic free test */
    {
        print("\nbasic free test\n\n");
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        free(ptr);
    }

    /* double free test */
    {
        print("\ndouble free test\n\n");
        int* ptr = (int*) malloc(sizeof(int));
        free(ptr);
        free(ptr);
    }

    /* multiple blocks test */
    {
        print("\nmultiple blocks test\n\n");
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
        print("\nzero bytes malloc\n\n");
        void* ptr = malloc(0);
        assert(ptr == NULL || ptr != NULL);
        free(ptr);
    }

    /* realloc with increasing size */
    {
        print("\nrealloc with increasing size\n\n");
        int* ptr = (int*) malloc(sizeof(int) * 2);
        ptr[0] = 10;
        ptr[1] = 20;
        ptr = (int*) realloc(ptr, sizeof(int) * 4);
        assert(ptr != NULL); 
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
        print("\nrealloc with decreasing size\n\n");
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
        print("\nrealloc with null pointers\n\n");
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
        print("\nrealloc with 0 size\n\n");
        int* ptr = (int*) malloc(sizeof(int) * 2);
        ptr[0] = 10;
        ptr[1] = 20;

        ptr = (int*) realloc(ptr, 0);
        assert(ptr == NULL);

        free(ptr);
    }

    /* very big malloc */
    {
        print("\nvery big malloc\n\n");
        size_t large_size = 1024 * 1024 * 1024;
        void* ptr = malloc(large_size);
        assert(ptr != NULL);
        free(ptr);
    }

    /* malloc unitialize memory */
    {
        print("\nmalloc unitialize memory\n\n");
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        free(ptr);
    }

    /* multiple malloc and free in random order */
    {
        print("\nmultiple malloc and free in random order\n\n");
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

    /* hex_dump function */
    {
        print("\nhex_dump function\n\n");
        size_t sizes[] = {16, 32, 64, 128, 256, 1024, 4096};

        for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
            size_t size = sizes[i];

            void *buffer = malloc(size);
            assert(buffer != NULL);

            for (size_t j = 0; j < size; j++) {
                ((unsigned char*)buffer)[j] = (unsigned char)(j % 256);
            }
            hex_dump();
            free(buffer);
        }
    }

    /* show_alloc_mem() function */
    {
        print("\nshow_alloc_mem() function\n\n");
        #define M (1024 * 1024)

        void* a = malloc(1);
        void* b = malloc(2);
        void* c = malloc(4);
        void* d = malloc(8);
        void* e = malloc(16);
        void* f = malloc(32);
        void* g = malloc(64);
        void* h = malloc(128);
        void* i = malloc(256);
        void* j = malloc(512);
        void* k = malloc(1024);
        void* l = malloc(1024 * 2);
        void* m = malloc(1024 * 4); 
        void* n = malloc(1024 * 32);
        void* o = malloc(M);
        void* p = malloc(16*M);
        void* q = malloc(128*M);
        show_alloc_mem(); 

    }
}