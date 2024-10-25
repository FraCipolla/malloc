#include <unistd.h>
#include <stdio.h>
#include "malloc.h"

char *buff = "stringa di copia da copiare";

int main() {
    char *t = malloc(100);
    int i = 0;
    while (buff[i]) {
        t[i] = buff[i++];
    }
    t[i + 1] = 0;
    printf("%s\n", t);
    char *t2 = malloc(100);
    i = 0;
    while (buff[i]) {
        t2[i] = buff[i++];
    }
    t2[i + 1] = 0;
    printf("%s\n", t2);
}