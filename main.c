#include <unistd.h>
#include <stdio.h>

int main() {
    int page = getpagesize();
    printf("size: %d\n", page);
    printf("%d\n", 1 << 10);
    printf("%d\n", 1 << 6);
}