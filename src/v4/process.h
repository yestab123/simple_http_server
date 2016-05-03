#define MAXEVENT 5000

struct event_s
{
    int fd;
    uint32_t event;
};

struct eventlist_s
{
    int count;
    struct event_s event[MAXEVENT];
};

struct PROCESS_STATIC
{
    int epoll_fd;
    int socket_keep;
    int connect_count;
    double rate;

    struct eventlist_s eventlist;
};
struct PROCESS_STATIC cs;

struct connect_link
{
    http_request *p;
    struct connect_link *next;
};
struct connect_link *all=NULL;
