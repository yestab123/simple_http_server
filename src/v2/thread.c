int thread_init(int num){
  int i;
  for(i=0;i<num;i++){
    pthread_t * p;
    pthread_create(p,NULL,thread_process,NULL);
  }
}

void * thread_process(void *arg){
  int thread_count=0;
  int accept_keep=0;
  int i;
  int e_fd;
  int count;
  int time_out=2000;//ms
  struct epoll_event EVENTS[MAXEVENTS];

  while(1){
   if((double)((thread_count*1.0)/(status.connect_count * 1.0)) < (status.rate))
    {
      if(1!=accept_keep)
	{
	  i=pthread_mutex_trylock(&accept_lock);
	  if(i==0){
	    accept_keep=1;
	    epoll_add(e_fd,sockfd);
	  }
	}
    }
   else{
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
	 set_non_blocking(connfd);
	 epoll_add(e_fd,connfd);
	 thread_count++;
	 status.connect_count++;
       }
     }
     else if(EVENTS[i].events & EPOLLIN){
       int work_fd=EVENTS[i].data.fd;
       process(work_fd);
     }

     else if(EVENTS[i].events & EPOLLOUT){
       work_fd=EVENTS[i].data.fd;
     }
   }

  }

}


 
