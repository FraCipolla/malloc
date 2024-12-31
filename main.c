#include <unistd.h>
#include <stdio.h>
#include "malloc.h"

void p(char *s) {
    printf("%s\n", s);
}

int main() {
    printf("sizeof t_header: %ld\n", sizeof(block_t));
    printf("Alignment: %ld\n", ALIGN(sizeof(block_t)));
    char *t = ft_malloc(100);
    
    char *buff = "stringa di copia da copiare";
    int i = 0;
    while (buff[i]) {
        t[i] = buff[i];
        i++;
    }
    t[i + 1] = '\0';
    ft_free(t);
    // realloc
    // t = realloc(t, 150);
    // printf("%s\n", t);
    // t = realloc(t, 150);
    // printf("%s\n", t);
    // t = realloc(t, 300);
    // printf("%s\n", t);

    // free
    for (int i = 0; i < 50; i++) {
        char *tmp = ft_malloc(100);
        tmp[0] = 'a';
    }

    for (int i = 0; i < 50; i++) {
        char *tmp = ft_malloc(500);
        tmp[0] = 'a';
    }

    for (int i = 0; i < 50; i++) {
        char *tmp = ft_malloc(5000);
        tmp[0] = 'a';
    }

    show_alloc_memory();
}