#include <unistd.h>

int main() {
  char out[14] = "Hello, World\n";
  write(1, out, sizeof(char)*14);

  return 0;
}
