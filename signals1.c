#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main() {
    pid_t pid = fork();

    if (pid == 0){ 
        printf("Child: Hello\n");
        printf("Child process group ID = %d\n", getpgrp());
        pause();   
    }
    else {           
        sleep(3);    
        kill(pid, SIGKILL); 
    }

    return 0;
}
