#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]){
        int first_fd;
        int second_fd;
        char arr[1024];
        ssize_t n;
        int digits = 0;
        int letters = 0;
        pid_t p;
        char out[200];
        int size;
        int i;

        if(argc != 3){
                printf("usage: %s file1 file2\n", argv[0]);
                exit(1);
        }

        second_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(second_fd < 0){
                perror("error opening second file");
                exit(1);
        }

        p = fork();
        if(p < 0){
                perror("error creating child");
                close(second_fd);
                exit(1);
        }

        if(p == 0){
                first_fd = open(argv[1], O_RDONLY);
                if(first_fd < 0){
                        perror("child error opening first file");
                        close(second_fd);
                        exit(1);
                }

                while((n = read(first_fd, arr, sizeof(arr))) > 0){
                        for(i = 0; i < n; i++){
                                if(isalpha((unsigned char)arr[i])){
                                        letters++;
                                }
                        }
                }

                if(n < 0){
                        perror("child error reading first file");
                        close(first_fd);
                        close(second_fd);
                        exit(1);
                }

                size = sprintf(out, "Child PID %d counted %d alphabets.\n", getpid(), letters);
                if(write(second_fd, out, size) != size){
                        perror("child error writing second file");
                        close(first_fd);
                        close(second_fd);
                        exit(1);
                }

                if(close(first_fd) < 0){
                        perror("child error closing first file");
                        close(second_fd);
                        exit(1);
                }

                if(close(second_fd) < 0){
                        perror("child error closing second file");
                        exit(1);
                }

                exit(0);
        }

        first_fd = open(argv[1], O_RDONLY);
        if(first_fd < 0){
                perror("parent error opening first file");
                close(second_fd);
                exit(1);
        }

        while((n = read(first_fd, arr, sizeof(arr))) > 0){
                for(i = 0; i < n; i++){
                        if(isdigit((unsigned char)arr[i])){
                                digits++;
                        }
                }
        }

        if(n < 0){
                perror("parent error reading first file");
                close(first_fd);
                close(second_fd);
                exit(1);
        }

        if(close(first_fd) < 0){
                perror("parent error closing first file");
                close(second_fd);
                exit(1);
        }

        if(wait(NULL) < 0){
                perror("parent error waiting for child");
                close(second_fd);
                exit(1);
        }

        size = sprintf(out, "Parent PID %d counted %d digits.\n", getpid(), digits);
        if(write(second_fd, out, size) != size){
                perror("parent error writing second file");
                close(second_fd);
                exit(1);
        }

        if(close(second_fd) < 0){
                perror("parent error closing second file");
                exit(1);
        }

        return 0;
}
