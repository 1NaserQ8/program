#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
        int fd;
        char c1,c2,c3;

        if(argc != 2){
                printf("usage: %s output_file\n", argv[0]);
                exit(1);
        }

        fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd < 0){
                perror("error opening output file");
                exit(1);
        }

        if(dup2(fd, 1) < 0){
                perror("error redirecting stdout");
                close(fd);
                exit(1);
        }

        close(fd);

        scanf("%c%c%c", &c1, &c2, &c3);
        printf("c1= %c , c2= %c , c3= %c \n", c1, c2, c3);

        return 0;
}
