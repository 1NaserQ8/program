#include <stdio.h>
#include <signal.h>

int main() {
    sigset_t set;
    int a, b;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    sigprocmask(SIG_BLOCK, &set, NULL);

    printf("Enter the value of a (cannot be interrupted): ");
    scanf("%d", &a);
    printf("The value of a = %d\n", a);

    sigprocmask(SIG_UNBLOCK, &set, NULL);

    printf("Enter the value of b (can be interrupted): ");
    scanf("%d", &b);
    printf("The value of b = %d\n", b);

    return 0;
}
