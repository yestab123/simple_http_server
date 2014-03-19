
void connect_shutdown(int fd,struct connect_status *cli){
  cli->phase=CLOSE_PHASE;
  fork_status.connect_count--;
  shutdown(fd,SHUT_RDWR);
}

void * thread_process(void *arg){
  int i;
  int RUNNING=1;
  int fd;
  while(RUNNING){
    i=pthread_mutex_lock(&fork_status.FD_LOCK);
    if(i!=0)
      {
	log_write("pthread_mutex_lock error",__FILE__,__LINE__,LOG_ERROR);
	continue;
      }
    else{
      i=read(fork_status.FD_LIST[0],&fd,sizeof(int));
      pthread_mutex_unlock(&fork_status.FD_LOCK);
    }
    switch(cli[fd].phase)
      {
      case ACCEPT_DATA_PHASE:
	i=recv_data(fd,&cli[fd]);
	if(-1==i)
	  {
	    connect_shutdown(fd,&cli[fd]);
	  }
	else{
	  i=process(fd,&cli[fd]);
	  if(i==0)
	    {
	      cli[fd].phase=DATA_PROCESS_PHASE;
	    }
	}
	break;
      case DATA_PROCESS_PHASE:
	i=method_process(fd,&cli[fd]);
	if(-1==i)
	  {
	    connect_shutdown(fd,&cli[fd]);
	  }
	get_process(fd,&cli[fd]);
	file_type_process(fd,&cli[fd]);
	pack_process(fd,&cli[fd]);
	cli[fd].phase=WAIT_SEND_PHASE;
	log_write("DONE",__FILE__,__LINE__,LOG_DEBUG);
	epoll_mod(epoll_fd,fd,EPOLLIN|EPOLLOUT);
	break;
      case WAIT_SEND_PHASE:
	send_process(fd,&cli[fd]);
	break;
      }
  }
}




int thread_init(int num){
  int i;
  int s;
  for(i=0;i<num;i++){
    pthread_t t;
    s=pthread_create(&t,NULL,thread_process,NULL);
    if(s!=0){
      log_write("create thread error",__FILE__,__LINE__,LOG_ERROR);
    }
  }
}

void *log_start(void *arg){
  log_file_record();
}
