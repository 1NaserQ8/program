#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
  int i;

  for (i=0; i<2; i++)
    fork();

  printf("Hello\n");

  return 0;
}

