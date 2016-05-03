#include "config.h"

extern unsigned int fork_mark[PROCESS_NUM];
extern int process();

static void child_restart()
{
    unsigned int i;
    i=wait(NULL);
    int j;
    if(i==-1)
        {
            printf("Wait() ERROR#\n");
        }
    else
        {
            for(j=0;j<PROCESS_NUM;j++)
                {
                    if(fork_mark[j]==i)
                        {
                            fork_id=j;
                            int x=fork();
                            {
                                if(x==0)
                                    {
                                        process();
                                        exit(0);
                                    }
                                else if(x<0)
                                    {
                                        printf("restart FORK error\n");
                                        exit(0);
                                    }
                                else
                                    {
                                        printf("one CHILD alread restart\n");
                                        fork_mark[j]=x;
                                        break;
                                    }
                            }
                        }
                }
        }
}

static void signal_handler(int signal)
{
    if(signal==SIGCHLD)
        {
            printf("One Child down#######\n");
            child_restart();
        }
    
}



void signal_add(int signal)
{
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler=signal_handler;
    sa.sa_flags|=SA_RESTART;
    sigfillset(&sa.sa_mask);
    int i=sigaction(signal,&sa,NULL);
    if(i<0)
        {
            printf("Add signal error#%d\n",signal);
            exit(0);
        }
    else
        {
            printf("Add signal::%d\n",signal);
        }
}
