

void * thread_process(void *arg){
  int thread_count=0;
  int accept_keep=0;
  int i;
  int e_fd;
  int count;
  int time_out=5000;//ms
  struct epoll_event EVENTS[MAXEVENTS];
  float rate_t=0.1;
#if DEBUG_T
  printf("thread_work\n");
#endif
  e_fd=epoll_create(10);
  set_non_block(e_fd);
  while(1){
   if(rate_t < (status.rate))
    {
      if(1!=accept_keep)
	{
	  i=pthread_mutex_trylock(&accept_lock);
	  if(i==0){
	    accept_keep=1;
	    epoll_add(e_fd,sockfd);
#if DEBUG_T
	    printf("#Get lock#\n");
#endif
	  }
	}
    }
   else if(1==accept_keep){
	epoll_del(e_fd,sockfd);
	pthread_mutex_unlock(&accept_lock);
	accept_keep=0;
      }

   if((0==thread_count)&&(0==accept_keep))
     {
       continue;
     }
   count=0;
   count=epoll_wait(e_fd,EVENTS,MAXEVENTS,time_out);
   if(-1==count)
     {
       continue;}
   for(i=0;i<count;i++){
     int fd=EVENTS[i].data.fd;
     if(1==accept_keep){
       if(fd==sockfd){
	 int connfd=accept(sockfd,NULL,NULL);
	 set_non_block(connfd);
	 epoll_add(e_fd,connfd);
	 thread_count++;
	 status.connect_count++;
	 connect_init(fd,&cli[connfd]);
	 cli[connfd].phase=ACCEPT_DATA_PHASE;
	 continue;
       }
     }
     if(EVENTS[i].events & EPOLLIN){
       int work_fd=EVENTS[i].data.fd;
       if(cli[work_fd].phase==ACCEPT_DATA_PHASE){
	 recv_data(work_fd,&cli[work_fd]);
	 process(work_fd,&cli[work_fd]);
	 method_process(work_fd,&cli[work_fd]);
	 get_process(work_fd,&cli[work_fd]);
	 file_type_process(work_fd,&cli[work_fd]);
	 pack_process(work_fd,&cli[work_fd]);
	 
	 cli[work_fd].phase==WAIT_SEND_PHASE;
       }
     }

     else if(EVENTS[i].events & EPOLLOUT){
       int work_fd;
       work_fd=EVENTS[i].data.fd;
       if(cli[work_fd].phase==WAIT_SEND_PHASE){
	 send_data(work_fd,&cli[work_fd]);
	 cli[work_fd].phase=DONE_PHASE;
       }
     }
   }

  }

}


void thread_init(int num){
  int i;
  int s;
  for(i=0;i<num;i++){
    pthread_t t;
    s=pthread_create(&t,NULL,thread_process,NULL);
    if(s!=0){
      printf("##create thread ERROR\n");
    }
  }
}

