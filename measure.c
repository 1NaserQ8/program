#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define N 1000

void initialize(int a[N][N], int n);

int main () {
  struct timeval tempo1, tempo2;
  long elapsed_utime;
  long elapsed_useconds;
  double t = 0.0;
  int a[N][N], i, j, tmp;

  initialize(a, 1);

  gettimeofday(&tempo1, NULL);
  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
      tmp = a[i][j];
  gettimeofday(&tempo2, NULL);
  elapsed_useconds = tempo2.tv_usec - tempo1.tv_usec;
  printf("Time for accessing a[i][j] is %ld microseconds\n", elapsed_useconds);

  gettimeofday(&tempo1, NULL);
  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
      tmp = a[j][i];
  gettimeofday(&tempo2, NULL);
  elapsed_useconds = tempo2.tv_usec - tempo1.tv_usec;
  printf("Time for accessing a[j][i] is %ld microseconds\n", elapsed_useconds);

  return 0;
}

void initialize(int a[N][N], int n) {
  int i, j;
  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
      a[i][j] = n;
}
