#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

int main(){
        int fd;
        int fd2;
        char buffer[1024];
        ssize_t bytes_read;
        struct stat s;

        fd = open("first_time.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if(fd < 0){
                perror("error with opening first_time.txt");
                exit(1);
        }

        while((bytes_read = read(0, buffer, sizeof(buffer))) > 0){
                if(write(fd, buffer, bytes_read) != bytes_read){
                        perror("error cant write file");
                        close(fd);
                        exit(1);
                }
        }

        if(bytes_read < 0){
                perror("error cant read input");
                close(fd);
                exit(1);
        }

        if(close(fd) < 0){
                perror("error closing first_time.txt");
                exit(1);
        }

        fd2 = open("Exists.txt", O_RDONLY);
        if(fd2 < 0){
                perror("error opening Exists.txt");
                exit(1);
        }

        fd = open("first_time.txt", O_WRONLY | O_APPEND);
        if(fd < 0){
                perror("error opening first_time.txt for append");
                close(fd2);
                exit(1);
        }

        while((bytes_read = read(fd2, buffer, sizeof(buffer))) > 0){
                if(write(fd, buffer, bytes_read) != bytes_read){
                        perror("error appending to first_time.txt");
                        close(fd2);
                        close(fd);
                        exit(1);
                }
        }

        if(bytes_read < 0){
                perror("error reading Exists.txt");
                close(fd2);
                close(fd);
                exit(1);
        }

        if(close(fd2) < 0){
                perror("error closing Exists.txt");
                close(fd);
                exit(1);
        }

        if(close(fd) < 0){
                perror("error closing first_time.txt");
                exit(1);
        }

        if(stat("first_time.txt", &s) < 0){
                perror("error with stat");
                exit(1);
        }

        printf("file size = %ld bytes\n", s.st_size);
        printf("file type and protections = %o\n", s.st_mode);
        printf("last modified = %s", ctime(&s.st_mtime));

        return 0;
}
