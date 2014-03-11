#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <strings.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sendfile.h>

#define OK 0
#define MAXEVENTS 65535
#define MAXSIZE 1500

#include "file.h"
#include "main.h"
#include "process.c"
#include "thread.c"

void set_non_block(int fd){
  int option=fcntl(fd,F_GETFL);
  option=option|O_NONBLOCK;
  fcntl(fd,F_SETFL,option);
}

int epoll_add(int e_fd,int fd){
  struct epoll_event event;
  int i;
  event.data.fd=fd;
  event.events=EPOLLIN|EPOLLRDHUP;
  i=epoll_ctl(e_fd,EPOLL_CTL_ADD,fd,&event);
  if(i!=0)
    {
      printf("epoll_add error #%d\n",e_fd);
    }
  return OK;
}

int epoll_del(int e_fd,int fd){
  int i;
  i=epoll_ctl(e_fd,EPOLL_CTL_DEL,fd,0);
  if(i!=0){
    printf("epoll_del error #%d\n",e_fd);
  }
  return OK;
}

int epoll_mod(int e_fd,int fd,int ev){
  struct epoll_event event;
  int i;
  event.data.fd=fd;
  event.events=ev|EPOLLRDHUP;
  i=epoll_ctl(e_fd,EPOLL_CTL_MOD,fd,&event);
  if(i!=0){
    printf("epoll_mod error #%d:%d\n",e_fd,fd);
  }
  return OK;
}


int main(int argc,char **argv){
  if(argc<3){
    printf("usage:service server_ip server_port\n");
    exit(0);
  }
  
  int port=atoi(argv[2]);
  char *ip=argv[1];
  struct sockaddr_in ser;
  ser.sin_family=AF_INET;
  ser.sin_port=htons(port);
  ser.sin_addr.s_addr=inet_addr(ip);
  
  int sock;
  sock=socket(AF_INET,SOCK_STREAM,0);
  int i;
  i=bind(sock,(struct sockaddr*)&ser,sizeof(ser));
  if(i!=0){
    printf("##bind err##\n");
    exit(0);
  }
  //  sleep(15);
  set_non_block(sock);

  status.connect_count=0;
  status.rate=0.5;
  
  
  listen(sock,10);
  sockfd=sock;
#if DEBUG_T
  printf("Init Test\n");
#endif
  thread_init(PTHREAD_NUM);
#if DEBUG_T
  printf("thread init DONE\n");
#endif
  while(1){}
}
