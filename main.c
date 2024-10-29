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
    char *t2 = malloc(100);
    i = 0;
    while (buff[i]) {
        t2[i] = buff[i++];
    }
    t2[i + 1] = 0;
    p(t2);
    for (int i = 0; i < 100; i++) {
        char *tmp = malloc(150);
        tmp[0] = 'a';
        free(tmp);
    }
}