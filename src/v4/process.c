#include "config.h"
#include "process.h"


extern struct ALL_STATIC *status;
extern int epoll_add(int epoll_fd,int fd,int op);
extern int epoll_del(int epoll_fd,int fd);
extern int epoll_mod(int epoll_fd,int fd,int op);
extern int set_non_block(int fd);
extern int http_request_init(http_request *p,int fd,uint32_t event);
extern int http_process(struct http_request_s *p);

static int socket_pressure_test();
static int get_time();
static int event_process();
static int cs_init();
int 
process()
{
    int time_wait;
    int mark=0;
    cs_init();
    printf("fork_init\n");
    while(1)
        {
            socket_pressure_test();
            time_wait=get_time();
            if(cs.socket_keep==1 || cs.connect_count>0)
                {
                    int i,j;
                    struct epoll_event event[MAXEVENT];
                    i=epoll_wait(cs.epoll_fd,event,MAXEVENT,time_wait);
                    if(i==-1)
                        {
                            if(errno==EINTR)
                                continue;
                        }
                    else if(i>0)
                        {
                            int fd;
                            mark=0;
                            for(j=0;j<i;j++)
                                {
                                    fd=event[j].data.fd;
                                    if(fd==status->socket_fd)
                                        {
                                            int fd_new;
                                            while((fd_new=accept(fd,NULL,NULL))>0)
                                                {
                                                    //   printf("New connection\n");
                                                    set_non_block(fd_new);
                                                    cs.connect_count+=1;
                                                    epoll_add(cs.epoll_fd,fd_new,EPOLLIN|EPOLLET);
                                                    mark++;
                                                    if(mark>500)
                                                        {
                                                            break;
                                                        }
                                                }
                                        }
                                    else
                                        {
                                            //printf("1.CONNECTION EVENT\n");
                                            cs.eventlist.event[cs.eventlist.count].fd=fd;
                                            cs.eventlist.event[cs.eventlist.count].event=event[j].events;
                                            cs.eventlist.count++;
                                            epoll_del(cs.epoll_fd,fd);
                                        }
                                }
                        }
                    if(cs.socket_keep==1 && cs.eventlist.count>0)
                        {
                            //                            printf("2.PROCESS CONNECTION\n");
                            cs.socket_keep=0;
                            epoll_del(cs.epoll_fd,status->socket_fd);
                            pthread_mutex_unlock(&status->SOCKET_LOCK);
                            
                        }
                    if(cs.eventlist.count>0)
                        {
                            event_process();
                        }
                       
                }
        }
}

int 
socket_pressure_test()
{
    status->process_link[fork_id]=cs.connect_count;
    cs.rate=status->process_rate[fork_id];
    if(cs.rate>MAX_RATE)
        {
            if(cs.socket_keep==1)
                {
                    epoll_del(cs.epoll_fd,status->socket_fd);
                    pthread_mutex_unlock(&status->SOCKET_LOCK);
                    cs.socket_keep=0;
                }
        }
    else if(cs.socket_keep==0)
        {
            int i;
            i=pthread_mutex_trylock(&status->SOCKET_LOCK);
            if(i==0)
                {
                    cs.socket_keep=1;
                    epoll_add(cs.epoll_fd,status->socket_fd,EPOLLIN);
                    //               printf("GET FD ACCEPT\n");
                }
        }
}

http_request* connect_find(int fd,uint32_t event)
{
    if(all==NULL)
        {
            all=(struct connect_link *)malloc(sizeof(struct connect_link));
            all->next=NULL;
            all->p=(http_request *)malloc(sizeof(http_request));
            if(all->p==NULL)
                {
                    printf("ALL MALLOC ERROR\n");
                    return NULL;
                }
            http_request_init(all->p,fd,event);
            return all->p;
        }
    else
        {
            struct connect_link *q=all,*t=all;
            while(q!=NULL)
                {
                    if(q->p->fd==fd)
                        {
                            return q->p;
                        }
                    t=q;
                    q=q->next;
                }
            q=(struct connect_link *)malloc(sizeof(struct connect_link));
            if(q==NULL)
                {
                    printf("connect link error\n");
                    return NULL;
                }
            q->next=NULL;
            q->p=(http_request *)malloc(sizeof(http_request));
            if(q->p==NULL)
                {
                    printf("http_request malloc error\n");
                    return NULL;
                }
            t->next=q;
            http_request_init(q->p,fd,event);
            return q->p;
        }
}

int connect_clean(int fd)
{
    struct connect_link *p=all,*q=all;
    while(q!=NULL)
        {
            if(q->p->fd==fd)
                {
                    if(q==all)
                        {
                            all=all->next;
                        }
                    p->next=q->next;
                    free(q->p);
                    free(q);
                }
            p=q;
            q=q->next;
        }
}
int event_process()
{
    int i;
    int fd;
    http_request *p;
    uint32_t event;
    for(i=0;i<cs.eventlist.count;i++)
        {
            fd=cs.eventlist.event[i].fd;
            // printf("3.EVENT_PROCESS %d\n",fd);
            event=cs.eventlist.event[i].event;
            p=connect_find(fd,event);
            if(p==NULL)
                {
                    continue;
                }
            http_process(p);
            if(p->phase==HTTP_CLEAN)
                {
                    //                    printf("%d : CLEAN\n",p->fd);
                    close(p->fd);
                    connect_clean(p->fd);
                    cs.connect_count-=1;
                    //epoll_add(cs.epoll_fd,p->fd,EPOLLIN|EPOLLET);
                    continue;
                }
            else if(p->phase==HTTP_CLOSE)
                {
                    // printf("%d : CLOSE\n",p->fd);
                    cs.connect_count-=1;
                    close(p->fd);
                    connect_clean(p->fd);
                    continue;
                }
            if(p->phase==HTTP_SEND_DONE)
                {
                    printf("%d :CLEAN\n",p->fd);
                    close(p->fd);
                    cs.connect_count-=1;
                    connect_clean(p->fd);
                    //epoll_add(cs.epoll_fd,p->fd,EPOLLIN|EPOLLET);
                }
            else
                {
                    epoll_add(cs.epoll_fd,p->fd,EPOLLIN|EPOLLOUT);
                }
            
        }
    cs.eventlist.count=0;
}

int get_time()
{
    return 30;
}

int cs_init()
{
    cs.epoll_fd=epoll_create(1);
    cs.socket_keep=0;
    cs.connect_count=0;
    cs.rate=0.0;
    cs.eventlist.count=0;
}
