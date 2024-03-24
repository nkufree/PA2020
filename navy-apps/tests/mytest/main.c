#include <stdio.h>
#include <stdlib.h>

void print_argv(char* v) {
    printf("%s\n", v);
}

int main(int      argc,
   char    *argv[]) {
  int*a = (int*)malloc(10*sizeof(int));
	write(1, "Hello World!0\n", 13);
  for(int i = 0; i < 10; i++)
    a[i] = i;
  write(1, "Hello World!1\n", 14);
  for(int i = 0; i < 10; i++)
    printf("%d\n", a[i]);
	write(1, "Hello World!2\n", 14);
  for(int i = 0; i < argc; i++)
    print_argv(argv[i]);
  a[1] = 1;
  write(1, "Hello World!3\n", 14);
  return 0;
}

