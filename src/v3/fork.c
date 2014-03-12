int fork_status_init(){
  fork_status.connect_count=0;
  fork_status.press_rate=0.0;
  fork_status.sock_keep=0;
}

int fork_process(){
  int epoll_fd=epoll_create(10);
  struct epoll_event EVENTS[MAXEVENTS];
  fork_status_init();
  int SERVER_RUN=1;
  while(SERVER_RUN){
    if(fork_status.press_rate<status.rate){
      if(fork_status.sock_keep==0){
	
      }
    }
    else if(fork_status.sock_keep==1){
      
      fork_status.sock_keep=0;
    }

    
  
