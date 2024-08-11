#include <unistd.h>
#include <stdio.h>
#include "malloc.h"

int main() {
    int page = getpagesize();
    printf("size: %d\n", page);
    printf("%d\n", TINY);
    printf("%d\n", SMALL);
    printf("%d\n", LARGE);
}