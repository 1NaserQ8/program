#include <stdio.h>
#include <unistd.h>

void display(char []);

int main() {
  char out[100];

  printf("Enter a string: ");
  scanf("%s", out);
  printf("You typed: \n");
  display(out);
  printf("\n");

  return 0;
}

void display(char out[]) {
  // Write your code here
  // Don't use printf()
  // Hint: use write, and count the legnth of out[].

}
