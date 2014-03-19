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
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#define OK 0
#define MAXEVENTS 65535
#define MAXSIZE 1500


#include "main.h"
#include "fork.h"
#include "log.h"
#include "signal.c"
#include "log.c"
#include "fork.c"
#include "process.c"
#include "thread.c"

int set_non_block(int fd){
  int option=fcntl(fd,F_GETFL);
  option=option|O_NONBLOCK;
  fcntl(fd,F_SETFL,option);
}

int epoll_add(int e_fd,int fd){
  struct epoll_event event;
  int i;
  event.data.fd=fd;
  event.events=EPOLLIN|EPOLLET|EPOLLRDHUP;
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
  event.events=ev|EPOLLET|EPOLLRDHUP;
  i=epoll_ctl(e_fd,EPOLL_CTL_MOD,fd,&event);
  if(i!=0){
    printf("epoll_mod error #%d:%d\n",e_fd,fd);
  }
  return OK;
}

int ser_status_init(){
  status->connect_count=0;
  status->rate=0.5;
  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&status->ACCEPT_LOCK,&mattr);
  
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
  
  
  status=(struct ser_status *)mmap(NULL,sizeof(struct ser_status),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
  if(status==MAP_FAILED)
    {
      printf("MMAP ERROR\n");
      exit(0);
    }

  ser_status_init();

  log_msgid=msgget(IPC_PRIVATE,0666|IPC_CREAT);
  if(log_msgid<0){
    printf("log id init err\n");
    exit(0);
  }

  listen(sock,10);
  status->sock_fd=sock;
  pthread_t t;
  i=pthread_create(&t,NULL,log_start,NULL);
   i=3000;
  while(i>0){
  log_write("test",__FILE__,__LINE__,LOG_ERROR);
  log_write("test",__FILE__,__LINE__,LOG_MESSAGE);
  //log_write("test",__FILE__,__LINE__,LOG_DEBUG);
  log_write("test",__FILE__,__LINE__,LOG_STATUS);
  i--;
  }
#ifndef DEBUG_FORK
  int pid;
  for(i=0;i<PROCESS_NUM;i++){
    pid=fork();
    if(pid==0)
      goto child;
    else if(pid<0)
      log_write("fork error",__FILE__,__LINE__,LOG_ERROR);
    else
      continue;
  }
  while(1){}
#endif

 child:
  fork_process();
}
