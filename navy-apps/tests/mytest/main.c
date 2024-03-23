#include <stdio.h>
#include <stdlib.h>

int main() {
//   printf("Hello, world!\n");
  int*a = (int*)malloc(10*sizeof(int));
  write(1, "Hello World!\n", 13);
  return 0;
}