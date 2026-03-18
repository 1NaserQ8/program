#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

int main(){
        int fd;
        char buffer[1024];
        ssize_t bytes_read;
        struct stat s;

        fd = open("first_time.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if(fd<0){
                perror("error with opening file");
                exit(1);
        }
        while((bytes_read = read(0, buffer, sizeof(buffer))) > 0){
                if (write(fd, buffer, bytes_read) != bytes_read){
                        perror("error cant write file");
                        close(fd);
                        exit(1);
                }
        }
        if(bytes_read < 0){
                perror("error cant read input");
        }
        close(fd);

        stat("first_time.txt", &s);
        printf("file size = %ld bytes\n", s.st_size);
        printf("file type and protections = %o\n", s.st_mode);
        printf("last modified = %s", ctime(&s.st_mtime));

        return 0;
}
