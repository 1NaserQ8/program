#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    char cmd[] = "/bin/ls";

    char *argv[] = {"ls", "-l", NULL};
    char *envp[] = {NULL};

    printf("---------The beginning of the original program---------\n");

    if (execve(cmd, argv, envp) < 0)
    {
        perror("execve failed");
        exit(1);
    }

    return 0;
}

