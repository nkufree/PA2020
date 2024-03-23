#include <stdio.h>
#include <stdlib.h>

int main() {
  int*a = (int*)malloc(10*sizeof(int));
  printf("Hello, world!\n");
  a[1] = 1;
  write(1, "Hello World!\n", 13);
  return 0;
}