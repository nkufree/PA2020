#include <stdio.h>
#include <NDL.h>

int main()
{
  NDL_Init(0);
  uint32_t last, curr;
  last = NDL_GetTicks();
  while(1)
  {
    curr = NDL_GetTicks();
    if(curr - last >= 500)
    {
      printf("hello\n");
      last = curr;
    }
  }
  NDL_Quit();
  return 0;
}