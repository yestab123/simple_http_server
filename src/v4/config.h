#ifndef __H_NETWORK__
#define __H_NETWORK__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/mman.h>
#include <error.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#endif

#ifndef __H_FUNCTION__
#define __H_FUNCTION__
#define PROCESS_NUM 4
#define BUFFER_MAXSIZE 1000

#define HTTP_INIT 0
#define HTTP_RECV 1
#define HTTP_METHOD 2
#define HTTP_URI 3
#define HTTP_VERSION 4
#define HTTP_RECV_DONE 12
#define HTTP_PROCESS 13
#define HTTP_PROCESS_DONE 14
#define HTTP_SEND 20
#define HTTP_SEND_DONE 22
#define HTTP_CLEAN 30         //keep the connection,only remove request status.
#define HTTP_CLOSE 31         //close the connection and remove all status.

#define PROCESS_DONE 10



struct ALL_STATIC
{
    int socket_fd;
    int process_count;
    int process_link[PROCESS_NUM];//each process connect count
    int sum_link;
    double process_rate[PROCESS_NUM];
    pthread_mutex_t SOCKET_LOCK;
};


static double MAX_RATE=0.70;
int fork_id;


#endif


#ifndef __H_HTTP_STRUCT__
#define __H_HTTP_STRUCT__

typedef struct http_package_s
{
    char file_path[100];
    char buffer[1000];
    int sendfile;
    int buffer_start;
    int buffer_end;
    int buffer_pos;
    int sendfile_done;
    
    
}http_package;

typedef struct string_s
{
    char *name;
    int length;
}string;

typedef struct http_in_s
{
    char buffer[BUFFER_MAXSIZE];
    int max_size;
    int start;
    int end;
    int now;
    
    int ps;//process_status;
    
    string method;
    string uri;
    string version;

    int method_i;
    int version_i;
}http_in;

typedef struct http_out_s
{
    int http_status;
    int file_fd;
    struct stat file_stat;
    char content_type[20];
    int content_length;
    
}http_out;

typedef struct http_request_s
{
    http_in header_in;
    http_out header_out;
    http_package package_send;
    int fd;
    int phase;
    int time;
    int error_sum;
    uint32_t now_event;
    int (*recv_process)(int fd,struct http_request_s *http);
    int (*method_process)(struct http_request_s *http);
    int (*process_request)(struct http_request_s *http);
    int (*send_process)(struct http_request_s *http);
    int (*send)(struct http_request_s *http);
}http_request;

#endif
