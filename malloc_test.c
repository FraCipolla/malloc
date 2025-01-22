# include <string.h>
# include <sys/mman.h>
# include <string.h>
# include <unistd.h>
# include <stdio.h>

int main()
{
    int i;
    char *addr;
    i = 0;

    while (i < 100)
    {
        addr =  (char *)mmap(0, getpagesize() * 10, PROT_READ | PROT_WRITE, MAP_ANON
        | MAP_PRIVATE, -1, 0);
        if (addr == MAP_FAILED) {
            perror("mmap failed");
            return -1;
        }
        addr[4095] = 1;
        if (munmap(addr, getpagesize() * 10) == -1) {
            perror("munmap failed");
            return -1;
        }
        i++;
    }
    return 0;
}