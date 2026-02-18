#include <stdio.h>
#include <unistd.h>

int main() {
  int a, b;

  printf("Enter the first integer: ");
  scanf("%d", &a);
  printf("Enter the second integer: ");
  scanf("%d", &b);

   pid_t pid = fork();
   if (pid < 0) {
        printf("Fork failed!\n");
        return 1;
    }
    else if (pid == 0) {
        printf("Result %d - %d = %d\n", a, b, a - b);
    }
    else {
        wait(NULL);
        printf("Result %d + %d = %d\n", a, b, a + b);
    }

  return 0;
}

