#include "config.h"



struct ALL_STATIC *status;

int 
server_static_init(int socket_fd)
{
    fork_id=0;
    status=(struct ALL_STATIC *)mmap(NULL,sizeof(struct ALL_STATIC),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if(status==MAP_FAILED||status==NULL)
        {
            printf("ALL_STATIC MMAP INIT ERROR\n");
            exit(0);
        }
    status->process_count=PROCESS_NUM;
    int i;
    for(i=0;i<status->process_count;i++)
        {
            status->process_link[i]=0;
            status->process_rate[i]=0.0;
        }
    status->sum_link=0;
    status->socket_fd=socket_fd;
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&status->SOCKET_LOCK,&mattr);
    return 0;
}

int server_running_update()
{
    int i;
    status->sum_link=0;
    for(i=0;i<status->process_count;i++)
        {
            status->sum_link+=status->process_link[i];
        }
    for(i=0;i<status->process_count;i++)
        {
            status->process_rate[i]=((double)status->process_link[i])/((double)status->sum_link);
            if(status->process_rate[i] <0)
                {
                    status->process_rate[i]=0;
                }
            #ifdef DEBUG 
            printf("server%d:%lf",i,status->process_rate[i]);
            #endif
        }
    
}
