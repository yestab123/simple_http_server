#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char ** argv)
{
  char a[]={"this is a game"};
  char b[]={"here i go"};
  printf("a:%s\nb:%s\n",a,b);
  char c[100];
  sprintf(c,"%s%s\0",a,b);
  printf("c:%s\n",c);
  sprintf(c,"%s%s\0",c,a);
  printf("c2:%s\n",c);
}
