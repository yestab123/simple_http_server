#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc,char **argv)
{
  struct stat st_test;
  int i;
  i=stat(argv[1],&st_test);
  if(i!=0)
    printf("stat error\n");
  if(S_ISDIR(st_test.st_mode))
    printf("is dir\n");
  if(S_ISREG(st_test.st_mode))
    printf("is U_file\n");
}
