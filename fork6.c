#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
  int n;

  printf("Enter number of children (max 10): ");
  scanf("%d", &n);

  if (n < 1) n = 1;
  if (n > 10) n = 10;

  for (int i = 0; i < n; i++) {
    pid_t pid = fork();

    if (pid == 0) {
      printf("i am a child %d and my parent is %d\n", (int)getpid(), (int)getppid());
      _exit(0);
    }
  }

  for (int i = 0; i < n; i++) {
    int status;
    pid_t cpid = wait(&status);

    if (WIFEXITED(status)) {
      printf("Child with id %d exited with status %d\n", (int)cpid, WEXITSTATUS(status));
    } else {
      printf("Child with id %d exited abnormal\n", (int)cpid);
    }
  }

  return 0;
}


