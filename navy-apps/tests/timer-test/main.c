#include <stdio.h>
#include <sys/time.h>

int main()
{
  struct timeval last, curr;
  gettimeofday(&last, NULL);
  while(1)
  {
    gettimeofday(&curr, NULL);
    int timegap = (curr.tv_sec - last.tv_sec) * 2 
    + (curr.tv_usec - last.tv_usec) * 2 / 1000000;
    if(timegap >= 1)
    {
      printf("hello\n");
      last.tv_sec = curr.tv_sec;
      last.tv_usec = curr.tv_usec;
    }
  }
  return 0;
}