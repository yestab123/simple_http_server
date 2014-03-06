

void * thread_process(void *arg){
  int thread_count=0;
  int accept_keep=0;
  int i;
  int e_fd;
  int count;
  int time_out=-1;//ms
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
     if(EVENTS[i].events & EPOLLRDHUP){
       int work_fd=EVENTS[i].data.fd;
       epoll_del(e_fd,work_fd);
       close(work_fd);
       thread_count--;
       status.connect_count--;
       cli[work_fd].phase=CLOSE_PHASE;
     }
     else if(EVENTS[i].events & EPOLLIN){
       int work_fd=EVENTS[i].data.fd;
       if(cli[work_fd].phase==DONE_PHASE){
	 connect_init(work_fd,&cli[work_fd]);
	 cli[work_fd].phase=ACCEPT_DATA_PHASE;
       }

       if(cli[work_fd].phase==ACCEPT_DATA_PHASE){
	 i=recv_data(work_fd,&cli[work_fd]);
	 if(i==-1)
	   {
	     epoll_del(e_fd,work_fd);
	     thread_count--;
	     status.connect_count--;
	     close(work_fd);
	     cli[work_fd].phase=CLOSE_PHASE;
	     continue;
	   }
	 if(cli[work_fd].buffer_size!=0){
	   cli[work_fd].phase=DATA_PROCESS_PHASE;
	 }
       }

       if(cli[work_fd].phase==DATA_PROCESS_PHASE){
	 process(work_fd,&cli[work_fd]);
	 i=method_process(work_fd,&cli[work_fd]);
	 if(i==-1){
	   epoll_del(e_fd,work_fd);
	   thread_count--;
	   status.connect_count--;
	   close(work_fd);
	   cli[work_fd].phase=CLOSE_PHASE;
	 }
	 get_process(work_fd,&cli[work_fd]);
	 file_type_process(work_fd,&cli[work_fd]);
	 pack_process(work_fd,&cli[work_fd]);
	 
	 cli[work_fd].phase=WAIT_SEND_PHASE;
	 epoll_mod(e_fd,work_fd,EPOLLIN|EPOLLOUT);
	 /*
	 i=send_process(work_fd,&cli[work_fd]);

	 if(i==-1){
	   thread_count--;
	   status.connect_count--;
	   epoll_del(e_fd,work_fd);
	   close(work_fd);
	   cli[work_fd].phase=CLOSE_PHASE;
	   continue;
	 }
	 cli[work_fd].phase==DONE_PHASE;*/
       }
     }

     else if(EVENTS[i].events & EPOLLOUT){
       int work_fd;
       work_fd=EVENTS[i].data.fd;
       if(cli[work_fd].phase==WAIT_SEND_PHASE){
	 int i=send_process(work_fd,&cli[work_fd]);
	 if(i==-1){
	   epoll_del(e_fd,work_fd);
	   close(work_fd);
	   thread_count--;
	   status.connect_count--;
	   cli[work_fd].phase=CLOSE_PHASE;
	 }
	 else{
	   cli[work_fd].phase=DONE_PHASE;
	   // epoll_mod(e_fd,work_fd,EPOLLIN);
	   epoll_del(e_fd,work_fd);
	   close(work_fd);
	   thread_count--;
	   status.connect_count--;
	   cli[work_fd].phase=CLOSE_PHASE;
	 }
       }
     }
   }
   if(status.connect_count!=0){
   rate_t=(float)(((float)(thread_count *1.0))/((float)(status.connect_count*1.0)))*1.0;
   }
   else{
     rate_t=0;
   }
   if(rate_t<0)
     {
       rate_t=0;
     }
   // printf("###%f\n\n",rate_t);
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

