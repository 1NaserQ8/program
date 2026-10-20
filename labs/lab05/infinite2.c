#include <stdio.h>
#include <unistd.h>

int main() {
  pid_t child_pid;

  if ((child_pid = fork()) == 0) {
    while (1) ;
  }
  else {
    if ((child_pid = fork()) == 0) {
      while (1) ;
    }
    while (1) ;
  }

  return 0;
}

