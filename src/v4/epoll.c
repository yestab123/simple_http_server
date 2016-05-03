#include "config.h"

int epoll_add(int epoll_fd,int fd,int op)
{
    struct epoll_event event;
    event.data.fd=fd;
    event.events=op;
    int i;
    i=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
    if(i!=0)
        {
            printf("epoll add error:%d\n",fd);
        }
}

int epoll_mod(int epoll_fd,int fd,int op)
{
    struct epoll_event event;
    event.data.fd=fd;
    event.events=op;
    epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&event);
    int i;
    if(i!=0)
        {
            printf("epoll mod error:%d\n",fd);
        }
}

int epoll_del(int epoll_fd,int fd)
{
    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,NULL);
}

int set_non_block(int fd)
{
    int option=fcntl(fd,F_GETFL);
    option=option|O_NONBLOCK;
    fcntl(fd,F_SETFL,option);
}
