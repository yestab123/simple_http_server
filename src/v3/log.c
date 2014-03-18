int log_write(char * log,char *file,int line,int level){
  struct log_buf buff;
  time_t now;
  char time_now[30];
  int i;
  time(&now);
  strftime(time_now,30,"%Y%m%d_%H%M",localtime(&now));
  if(level==LOG_ERROR){
    sprintf(buff.buf,"%s - %s \n",time_now,log);
  }
  else{
    sprintf(buff.buf,"%s - %s|%s-%d \n",time_now,log,file,line);
  }
  buff.level=level;
  buff.size=strlen(buff.buf);
  
  i=msgsnd(log_msgid,&buff,sizeof(struct log_buf),IPC_NOWAIT);
  if(i<0){
    printf("log_msg is full,msg drop\n");
  }
}


int log_file_record(){
  struct stat dump_stat;
  struct stat status_stat;
  int dump_fd;
  int status_fd;
  int i;
  int LOG_RUNNING=1;
  i=stat(DUMP_LOG_FILE,&dump_stat);
  if(i!=0){
    dump_fd=open(DUMP_LOG_FILE,O_RDWR|O_CREAT|O_EXCL|O_APPEND);
    if(dump_fd<0)
      {
	printf("dump_file create error!#\n");
	exit(0);
      }
  }
  else{
    if(dump_stat.st_size > LOG_MAXSIZE){
      char new_name[100];
      char time_name[100];
      time_t now;
      time(&now);
      strftime(time_name,100,"%Y%m%d_%H%M",localtime(&now));
      sprintf(new_name,"dump_%s.log",time_name);
      i=rename(DUMP_LOG_FILE,new_name);
      if(i!=0)
	{
	  dump_fd=open(DUMP_LOG_FILE,O_RDWR|O_CREAT|O_TRUNC|O_APPEND);
	}
    }
    else{
      dump_fd=open(DUMP_LOG_FILE,O_RDWR|O_APPEND);
    }
  }
  
  i=stat(STATUS_LOG_FILE,&status_stat);
  if(i!=0){
    status_fd=open(STATUS_LOG_FILE,O_RDWR|O_CREAT|O_EXCL|O_APPEND);
    if(dump_fd<0)
      {
	printf("status_file create error!#\n");
	exit(0);
      }
  }
  else{
    if(status_stat.st_size > LOG_MAXSIZE){
      char new_name[100];
      char time_name[100];
      time_t now;
      time(&now);
      strftime(time_name,100,"%Y%m%d_%H%M",localtime(&now));
      sprintf(new_name,"dump_%s.log",time_name);
      i=rename(STATUS_LOG_FILE,new_name);
      if(i!=0)
	{
	  status_fd=open(STATUS_LOG_FILE,O_RDWR|O_CREAT|O_TRUNC|O_APPEND);
	}
    }
    else{
      dump_fd=open(DUMP_LOG_FILE,O_RDWR|O_APPEND);
    }
  }

  while(LOG_RUNNING){
    struct log_buf buff;
    int k;
    k=msgrcv(log_msgid,&buff,sizeof(struct log_buf),0,0);
    if(buff.level==LOG_DEBUG){
      printf("%s\n",buff.buf);
    }
    else if(buff.level==LOG_MESSAGE || buff.level==LOG_STATUS){
      write(status_fd,buff.buf,buff.size);
    }
    else if(buff.level==LOG_ERROR){
      write(dump_fd,buff.buf,buff.size);
    }
  }
  close(dump_fd);
  close(status_fd);
}
	    


