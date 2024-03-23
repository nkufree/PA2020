#include <stdio.h>
#include <stdlib.h>

int main() {
  printf("Hello, world!\n");
  int* a = malloc(sizeof(int));
  int *b[10] = {};
  for(int i = 0; i < 10; i++) {
    b[i] = malloc(sizeof(int));
    *b[i] = i;
  }
  for(int i = 0; i < 10; i++) {
    printf("%p, %d\n",b + i, *b[i]);
  }
  return 0;
}