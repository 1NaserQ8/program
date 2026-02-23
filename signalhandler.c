#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    printf("\ninside the handler function\n");
}

int main() {
    int i = 1;

    signal(SIGINT, handler); 

    while (1) {
        printf("%d : inside the main function\n", i++);
        sleep(1);
    }

    return 0;
}
