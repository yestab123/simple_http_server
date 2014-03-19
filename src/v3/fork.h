struct fork_status_s{
  pthread_mutex_t FD_LOCK;
  pthread_mutex_t ADD_LOCK;
  int connect_count;
  double press_rate;
  int sock_keep;
  int FD_LIST[2];
  int ADD_FD[2];
};

struct add_fd_status{
  int fd;
  int ev;
};

struct fork_status_s fork_status={.FD_LOCK=PTHREAD_MUTEX_INITIALIZER,\
				  .ADD_LOCK=PTHREAD_MUTEX_INITIALIZER};
int epoll_fd;
