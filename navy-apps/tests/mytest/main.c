#include <stdio.h>
#include <stdlib.h>

void print_argv(char* v) {
    write(1, v, strlen(v));
    write(1, "\n", 1);
}

int main(int      argc,
   char    *argv[]) {
  int*a = (int*)malloc(10*sizeof(int));
  for(int i = 0; i < argc; i++)
    print_argv(argv[i]);
  a[1] = 1;
  write(1, "Hello World!\n", 13);
  return 0;
}

