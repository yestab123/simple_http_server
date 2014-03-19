int fork_status_init(){
  fork_status.connect_count=0;
  fork_status.press_rate=0.0;
  fork_status.sock_keep=0;
  int i=pipe(fork_status.FD_LIST);
  int j=pipe(fork_status.ADD_FD);
  if(i!=0||j!=0)
    {
      log_write("pipe create error",__FILE__,__LINE__,LOG_ERROR);
    }
}

int load_balance_test(int count,int * load_rate){
  if(count>1000)
    {
      (*load_rate)++;
    }
  else{
    (load_rate)=0;
  }
    
}
  
int fork_process(){
  epoll_fd=epoll_create(10);
  struct epoll_event EVENTS[MAXEVENTS];
  fork_status_init();
  thread_init(PTHREAD_NUM);
  int SERVER_RUN=1;
  int i;
  int timeout=-1;
  int count;
  int load_rate=0;
  set_non_block(fork_status.ADD_FD[0]);
  epoll_add(epoll_fd,fork_status.ADD_FD[0]);
  while(SERVER_RUN){
    pthread_mutex_lock(&fork_status.COUNT_LOCK);
    count=fork_status.connect_count;
    pthread_mutex_unlock(&fork_status.COUNT_LOCK);
    load_balance_test(count,&load_rate);
    if( (count<FORK_MAX_CONNECTION) && (load_rate < 200)){
      if(fork_status.sock_keep==0){
	i=pthread_mutex_trylock(&status->ACCEPT_LOCK);
	if(i==0)
	  {
	    fork_status.sock_keep=1;
	    epoll_add(epoll_fd,status->sock_fd);
	  }
      }
    }
    else if(fork_status.sock_keep==1){
      epoll_del(epoll_fd,status->sock_fd);
      pthread_mutex_unlock(&status->ACCEPT_LOCK);
      load_rate=0;
      fork_status.sock_keep=0;
      log_write("ACEEPT_LOCK#relase",__FILE__,__LINE__,LOG_STATUS);
    }
    if(fork_status.sock_keep==0 && fork_status.connect_count==0){
      continue;
    }

    int count=epoll_wait(epoll_fd,EVENTS,MAXEVENTS,timeout);
    if(count<0){
      if(errno==EINTR)
	{
	  continue;
	}
      else{
	log_write("epoll_wait return -1",__FILE__,__LINE__,LOG_ERROR);
	exit(0);
      }
    }
    for(i=0;i<count;i++)
      {
	int fd=EVENTS[i].data.fd;
	if(1==fork_status.sock_keep)
	  {
	    if(fd==(status->sock_fd)){
	      int connfd=accept(fd,NULL,NULL);
	      set_non_block(connfd);
	      epoll_add(epoll_fd,connfd);
	      epoll_mod(epoll_fd,connfd,EPOLLIN|EPOLLONESHOT);
	      pthread_mutex_lock(&fork_status.COUNT_LOCK);
	      fork_status.connect_count++;
	      pthread_mutex_unlock(&fork_status.COUNT_LOCK);
	      connect_init(fd,&cli[connfd]);
	      continue;
	    }
	  }
	if(fd==fork_status.ADD_FD[0])
	  {
	    int i=1;
	    struct add_fd_status a_fd;
	    while(i>0)
	      {
		i=read(fd,&a_fd,sizeof(struct add_fd_status));
		//epoll_add(epoll_fd,a_fd.fd);
		epoll_mod(epoll_fd,a_fd.fd,a_fd.ev|EPOLLONESHOT);
	      }
	    continue;
	  }
	if(EVENTS[i].events & EPOLLRDHUP){
	  int work_fd=EVENTS[i].data.fd;
	  epoll_del(epoll_fd,work_fd);
	  connect_shutdown(work_fd,&cli[work_fd]);
	}

	else if(EVENTS[i].events & EPOLLIN)
	  {
	    int work_fd=EVENTS[i].data.fd;
	    //epoll_del(epoll_fd,work_fd);
	    write(fork_status.FD_LIST[1],&work_fd,sizeof(int));

	  }

	else if(EVENTS[i].events & EPOLLOUT)
	  {
	    int work_fd=EVENTS[i].data.fd;
	    //epoll_del(epoll_fd,work_fd);
	    write(fork_status.FD_LIST[1],&work_fd,sizeof(int));
	  }
      }
  }


}
  
