struct ser_status{
  int connect_count;
  double rate;
};

struct ser_status status;

struct rpc_t{
  char *way;
  char *path;
  char *version;
};

struct connect_status {
  int fd;
  int phase;
  int status;
  struct rpc_t rpc;
  char buffer[500];
  int buffer_size;
};


pthread_mutex_t  accept_lock;

#define CONNECT_INIT_PHASE 1
#define ACCEPT_DATA_PHASE 2
#define DATA_PROCESS_PHASE 3
#define WAIT_SEND_PHASE 4
#define DONE_PHASE 5
