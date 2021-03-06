#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"

int main(int argc,char **argv){
  if(argc<3)
    {
      printf("usage:packet PORT PATH/FILE\n");
      exit(0);
    }
  int ser_port=atoi(argv[1]);
  int sock_fd;
  sock_fd=socket(AF_INET,SOCK_STREAM,0);
  struct sockaddr_in sock_addr;
  sock_addr.sin_port=htons(ser_port);
  sock_addr.sin_family=AF_INET;
  sock_addr.sin_addr.s_addr=inet_addr(SERVER_IP);
  
  int i;
  i=connect(sock_fd,(struct sockaddr*)&sock_addr,sizeof(sock_addr));
  if(i!=0)
    {
      printf("##connect error\n");
      exit(0);
    }
  char buf[300];
  sprintf(buf,"GET /%s HTTP/1.1\r\n",argv[2]);
  send(sock_fd,buf,strlen(buf),0);
  char get[300];
  i=recv(sock_fd,get,300,0);
  if(i==0)
    {
      printf("##GET NOT THING\n");
      exit(0);
    }
  get[i]='\0';
  printf("\n--------------------\n\n%s\n\n",get);
}
