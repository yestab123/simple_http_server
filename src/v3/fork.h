struct fork_status_s{
  pthread_mutex_t FD_LOCK;
  int connect_count;
  double press_rate;
  int sock_keep;
  int FD_LIST[2];
  
};

struct fork_status_s fork_status={PTHREAD_MUTEX_INITIALIZER};
int epoll_fd;
