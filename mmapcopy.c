#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    char *buffer;
    int fd;
    struct stat info;
    size_t length;
    pid_t pid;
    int status;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(1);
    }
    
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    
    if (fstat(fd, &info) == -1) {
        perror("fstat");
        close(fd);
        exit(1);
    }
    
    if (info.st_size == 0) {
        fprintf(stderr, "%s is empty\n", argv[1]);
        close(fd);
        exit(1);
    }
    
    length = info.st_size;
    
    pid = fork();
    if (pid < 0) {
        perror("fork");
        close(fd);
        exit(1);
    }
    
    if (pid == 0) {
        printf("Child process\n");
        
        buffer = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
        if (buffer == MAP_FAILED) {
            perror("mmap");
            close(fd);
            exit(1);
        }
        
        if (write(STDOUT_FILENO, buffer, length) == -1) {
            perror("write");
            munmap(buffer, length);
            close(fd);
            exit(1);
        }
        
        munmap(buffer, length);
        close(fd);
        
        printf("\nCP: done\n");
        exit(0);
    }
    
    printf("Parent process\n");
    wait(&status);
    close(fd);
    printf("PP: done\n");
    
    return 0;
}
