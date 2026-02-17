#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
  int i;

  for (i=0; i<3; i++) {
    if (i==2)
      continue;
    fork();
  }

  printf("Hello\n");

  return 0;
}

