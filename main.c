#include "malloc.h"
#include <assert.h>

int main()
{
    /* basic malloc test */
    {
        print("\nbasic malloc test:\n");

        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        *ptr = 42;
        assert(*ptr == 42);
        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* basic free test */
    {
        print("\nbasic free test\n");
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* double free test */
    {
        print("\ndouble free test\n");
        int* ptr = (int*) malloc(sizeof(int));
        free(ptr);
        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* multiple blocks test */
    {
        print("\nmultiple blocks test\n");
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
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* zero bytes malloc */
    {
        print("\nzero bytes malloc\n");
        void* ptr = malloc(0);
        assert(ptr == NULL || ptr != NULL);
        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* realloc with increasing size */
    {
        print("\nrealloc with increasing size\n");
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
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* realloc with decreasing size */
    {
        print("\nrealloc with decreasing size\n");
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
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* realloc with null pointers */
    {
        print("\nrealloc with null pointers\n");
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
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* realloc with 0 size */
    {
        print("\nrealloc with 0 size\n");
        int* ptr = (int*) malloc(sizeof(int) * 2);
        ptr[0] = 10;
        ptr[1] = 20;

        ptr = (int*) realloc(ptr, 0);
        assert(ptr != NULL);

        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* very big malloc */
    {
        print("\nvery big malloc\n");
        size_t large_size = 1024 * 1024 * 1024;
        void* ptr = malloc(large_size);
        assert(ptr != NULL);
        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* malloc unitialize memory */
    {
        print("\nmalloc unitialize memory\n");
        int* ptr = (int*) malloc(sizeof(int));
        assert(ptr != NULL);
        free(ptr);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }

    /* multiple malloc and free in random order */
    {
        print("\nmultiple malloc and free in random order\n");
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
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
    }


    /* calloc test */
    {
        print("\nCalloc test\n\n");

        size_t num_elements = 10;
        int* ptr = (int*)calloc(num_elements, sizeof(int));        

        assert(ptr != NULL);

        for (size_t i = 0; i < num_elements; ++i) {
            assert(ptr[i] == 0);
        }

        free(ptr);

        ptr = (int*)calloc(0, sizeof(int));

        assert(ptr != NULL);  
        free(ptr);

        ptr = (int*)calloc(num_elements, 0);

        assert(ptr != NULL);  
        free(ptr);

        uint64_t* ptr_64 = (uint64_t *)calloc(num_elements * 100, sizeof(uint64_t));
        free(ptr_64);
    }

    /* reallocarray test */
    {
        print("\nReallocarray test:\n");
        print("\t- Test case 1: Regular resizing to a larger size\n");
        size_t num_elements = 5;
        size_t size_per_element = sizeof(int);
        int* ptr = (int*)malloc(num_elements * size_per_element);

        for (size_t i = 0; i < num_elements; ++i) {
            ptr[i] = i;
        }

        size_t new_num_elements = 10;
        ptr = (int*)reallocarray(ptr, new_num_elements, size_per_element);

        assert(ptr != NULL);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");

    
        for (size_t i = 0; i < num_elements; ++i) {
            assert(ptr[i] == (int)i);
        }

        for (size_t i = num_elements; i < new_num_elements; ++i) {
            assert(ptr[i] == 0);
        }

        free(ptr);

        print("\t- Test case 2: Resize to a smaller size\n");
        ptr = (int*)malloc(num_elements * size_per_element);

        for (size_t i = 0; i < num_elements; ++i) {
            ptr[i] = i;
        }

        size_t smaller_num_elements = 3;
        ptr = (int*)reallocarray(ptr, smaller_num_elements, size_per_element);

        assert(ptr != NULL);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");

        for (size_t i = 0; i < smaller_num_elements; ++i) {
            assert(ptr[i] == (int)i);
        }

        free(ptr);

        print("\t- Test case 3: Resize with 0 elements\n");
        ptr = (int*)malloc(num_elements * size_per_element);
        ptr = (int*)reallocarray(ptr, 0, size_per_element);

        assert(ptr != NULL);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
        free(ptr);
        print("\t- Test case 4: Resize with 0 size per element\n");
        ptr = (int*)malloc(num_elements * size_per_element);
        ptr = (int*)reallocarray(ptr, num_elements, 0);

        assert(ptr != NULL);
        print("\033[0;32m");
        print("\t\ttest passed\n");
        print("\033[0m");
        free(ptr);
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
        free(a);    free(b);    free(c);
        free(d);    free(e);    free(f);
        free(g);    free(h);    free(i);
        free(j);    free(k);    free(l);
        free(m);    free(n);    free(o);
        free(p);    free(q);
    }

    /* hex_dump function */
    {
        print("\nhex_dump function\n\n");
        size_t sizes[] = {16, 32, 64, 128, 256, 1024};

        for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
            size_t size = sizes[i];

            void *buffer = malloc(size);
            assert(buffer != NULL);

            for (size_t j = 0; j < size; j++) {
                ((unsigned char*)buffer)[j] = (unsigned char)(j % 256);
            }
            // hex_dump();
            free(buffer);
        }
    }

    /* print_memory() function */
    {
        print("\nprint_memory() function\n\n");
        print_memory();
    }

    /* show_alloc_mem_ex() function */
    // {
    //     print("\nshow_alloc_mem_ex() function\n\n");
    //     show_alloc_mem_ex();
    // }

    /* display defragmentation */
    {
        int *ptr = calloc(42, sizeof(int));
        int *ptr2 = calloc(42, sizeof(int));
        int *ptr3 = calloc(42, sizeof(int));
        int *ptr4 = calloc(42, sizeof(int));
        int *ptr5 = calloc(42, sizeof(int));
    
        for (int i = 0; i < 42; i++) {
            ptr[i] = 42;
            ptr2[i] = 42;
            ptr3[i] = 42;
            ptr4[i] = 42;
            ptr5[i] = 42;
        }

        print("Allocate 5 blocks blocks\n");
        print_memory();

        free(ptr2);
        free(ptr4);

        print("Block 1 and 3 are being freed\n");
        print_memory();

        free(ptr3);

        print("Block 2 is being freed, all blocks are merged\n");
        print_memory();

        free(ptr5);

        print("Last block is being freed, only block 1 remain\n");
        print_memory();
        
        free(ptr);

        print("All blocks are freed, only the chunk header remains.\n");
        print_memory();
    }
}