struct ser_status{
  int connect_count;
  double rate;
};

struct ser_status status;

struct rpc_t{
  char *method;
  char *path;
  char *version;
  
  int method_i;
  
};

/*
struct data_cache{
  union{
    char * point;
    char buffer[MAXSIZE];
  }
  struct data_cache *next;
  int size;
  int choose;//POINT,BUFFER
};
*/

struct connect_status {
  int fd;
  int phase;
  int status;
  struct rpc_t rpc;

  char buffer[MAXSIZE];
  int buffer_size;

  char send[MAXSIZE];
  //struct data_cache *cache;
  int send_size;
  int send_done;
};

struct connect_status cli[20000];


pthread_mutex_t  accept_lock;

#define HOME_DIR "/Project/web/"

#define POINT_U 1
#define BUFFER_U 2

#define CLOSE -5

#define NON_CONNECT 0
#define CONNECT_INIT_PHASE 1
#define ACCEPT_DATA_PHASE 2
#define DATA_PROCESS_PHASE 3
#define WAIT_SEND_PHASE 4
#define DONE_PHASE 5


#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_HEAD 3
