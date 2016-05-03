#include "config.h"
extern int process();
extern int server_static_init(int socket_fd);
extern int server_running_update();
extern int set_non_block(int fd);
extern void signal_add(int signal);

unsigned int fork_mark[PROCESS_NUM];
int main(int argc,char **argv)
{
    if(argc<3)
        {
            printf("usage:server [SERVER_IP] [SERVER_PORT]\n");
            exit(0);
        }
    int port=atoi(argv[2]);
    char *ip=argv[1];
    struct sockaddr_in ser;
    ser.sin_addr.s_addr=inet_addr(ip);
    ser.sin_family=AF_INET;
    ser.sin_port=htons(port);
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    int i=bind(sockfd,(struct sockaddr*)&ser,sizeof(ser));
    if(i!=0)
        {
            printf("#BIND PORT ERROR#\n");
            exit(0);
        }
    signal_add(SIGCHLD);
    set_non_block(sockfd);
    server_static_init(sockfd);
    listen(sockfd,5);
    int t;
    for(i=0;i<PROCESS_NUM;i++)
        {
            t=fork();
            if(t==0)
                {
                    process();
                    exit(0);
                }
            else if(t<0)
                {
                    printf("#fork error#");
                    exit(0);
                }
            else
                {
                    fork_mark[fork_id]=t;
                    fork_id++;
                }
        }
    while(1)
        {
            server_running_update();
        }
}
