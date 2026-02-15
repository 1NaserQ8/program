#include <stdio.h>

int main() {
  int *ptr1, *ptr2;
  int arr[2] = {1,2};

  ptr1 = &arr[0];
  ptr2 = 0x1234;

  printf("*ptr1 = %d\n", *ptr1);
  printf("*(ptr1+1) = %d\n", *(ptr1+1));
  printf("*(ptr1+2) = %d\n", *(ptr1+2));
  printf("*ptr2 = %d\n", *ptr2);

  return 0;
}
