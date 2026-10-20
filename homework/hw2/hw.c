#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("hi.txt", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    fstat(fd, &st);
    size_t filesize = st.st_size;

    char *map = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    memcpy(map, "hello universe\n", strlen("hello universe\n"));

    msync(map, filesize, MS_SYNC);

    write(STDOUT_FILENO, map, filesize);

    munmap(map, filesize);
    close(fd);
    return 0;
}
