#include <unistd.h>
#include <stdio.h>
#include "malloc.h"

char *buff = "stringa di copia da copiare";

void p(char *s) {
    printf("%s\n", s);
}

int main() {
    char *t = malloc(100);
    int i = 0;
    while (buff[i]) {
        t[i] = buff[i++];
    }
    t[i + 1] = 0;
    p(t);
    
    // realloc
    t = realloc(t, 150);
    printf("%s\n", t);
    t = realloc(t, 150);
    printf("%s\n", t);
    t = realloc(t, 300);
    printf("%s\n", t);

    // free
    for (int i = 0; i < 150; i++) {
        char *tmp = malloc(100);
        tmp[0] = 'a';
        free(tmp);
    }
}