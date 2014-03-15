struct fork_status_s{
  int connect_count;
  double press_rate;
  int sock_keep;
  
  pthread_mutex_t IN_LOCK;
  int IN_LIST[2];
  pthread_mutex_t OUT_LOCK;
  int OUT_LIST[2];
};

struct fork_status_s fork_status;

