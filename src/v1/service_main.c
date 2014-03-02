#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <error.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>


#include "service.h"
#include "signal.c"
#include "process.c"
#include "pthread.c"

#define OK 1
#define MAXEVENTS 40000
#define PTHREAD_NUM 4

int pipe_create(int pi[])
{
  int i;
  i=pipe(pi);
  if(i!=0)
    {
      printf("####create pipe error\n");
    }
}

int setnonblocking(int fd)
{
  int old_option=fcntl(fd,F_GETFL);
  int new_option=old_option|O_NONBLOCK;
  fcntl(fd,F_SETFL,new_option);
  return old_option;
}

int e_add(int e_fd,int fd)
{
  int i;
  struct epoll_event e_event;
  e_event.data.fd=fd;
  e_event.events=EPOLLIN|EPOLLET;
  if((i=epoll_ctl(e_fd,EPOLL_CTL_ADD,fd,&e_event))!=0)
    {
      printf("e_add() error!#\n");
    }
  return OK;
}

int e_del(int e_fd,int fd)
{
  int i;
  struct epoll_event e_event;
  e_event.data.fd=fd;
  e_event.events=EPOLLIN|EPOLLET;
  if((i=epoll_ctl(e_fd,EPOLL_CTL_DEL,fd,&e_event))!=0)
    {
      printf("e_del error#\n");
    }
  //printf("######FD#######||%d||###closed###\n",fd);
  close(fd);
  return OK;
}



int epoll_work_running(int sock_fd)
{
  int e_fd=epoll_create(MAXEVENTS);
  e_add(e_fd,sock_fd);
  e_add(e_fd,fd_clean[0]);
  struct epoll_event EVENTS[MAXEVENTS];
  int count;
  while(1)
    {
      count=epoll_wait(e_fd,EVENTS,MAXEVENTS,-1);
      if(count==-1)
	{
	  if(errno=EINTR)
	    {
	      continue;
	    }
	  else
	    break;
	}
      int i;
      for(i=0;i<count;i++)
	{
	  int getfd=EVENTS[i].data.fd;
	  if(getfd==sock_fd)
	    {
	      int connfd=accept(sock_fd,NULL,NULL);
	      setnonblocking(connfd);
	      e_add(e_fd,connfd);
	    }
	  else if(getfd==fd_clean[0])
	    {
	      int fd_c=0;
	      read(fd_clean[0],&fd_c,1);
	      printf("=============================\n##########GET SIGNAL::%d##\n==============\n\n",fd_c);
	      // read(fd_clean[0],&fd_c,4);
	      //e_del(e_fd,fd_c);
	    }
	  else if(EVENTS[i].events & EPOLLIN)
	    {
	      int work_fd=EVENTS[i].data.fd;/*
	      int status=process(work_fd);
	      if(status==5)
		{
		  e_del(e_fd,work_fd);
		  }*/
	      
	      //pipe_version
	      write(p_to_p[1],&work_fd,4);
	    }
	}
    }
  return 0;
}

      


int main(int argc, char **argv)
{
  if(argc<3)
    {
      printf("usage:service serIP serPORT\n");
      exit(0);
    }
  int service_port;
  char * service_ip;
  int service_sock_fd;
  service_port=atoi(argv[2]);
  service_ip=argv[1];
  service_sock_fd=socket(AF_INET,SOCK_STREAM,0);

  pipe_create(p_to_p);
  pipe_create(fd_clean);
  setnonblocking(fd_clean[1]);

  add_signal(SIGPIPE);

  struct sockaddr_in service_sockaddr;
  service_sockaddr.sin_family=AF_INET;
  service_sockaddr.sin_port=htons(service_port);
  service_sockaddr.sin_addr.s_addr=inet_addr(service_ip);
  
  int i;
  i=bind(service_sock_fd,(struct sockaddr*)&service_sockaddr,sizeof(service_sockaddr));
  if(i!=0){
    printf("bind error!!\n");
    exit(0);
  }
  
  setnonblocking(service_sock_fd);
  i=listen(service_sock_fd,10);
  if(i!=0)
    {
      printf("listen error!!\n");
      exit(0);
    }

  init_pthread(PTHREAD_NUM);

  epoll_work_running(service_sock_fd);
  close(service_sock_fd);
}
