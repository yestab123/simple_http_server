int connect_init(int fd,struct connect_status *cli){
  cli->fd=fd;
  cli->phase=CONNECT_INIT_PHASE;
  cli->status=404;
  cli->rpc.method=NULL;
  cli->rpc.path=NULL;
  cli->rpc.version=NULL;
  cli->rpc.blank=0;
  cli->rpc.rpc_done=0;
  cli->rpc.read_set=0;
  cli->rpc.mark=0;
  memset(cli->buffer,'\0',MAXSIZE);
  cli->buffer_size=0;
  memset(cli->send,'\0',MAXSIZE);
  cli->send_size=0;
  cli->send_done=0;
  memset(&cli->file_stat,'\0',sizeof(struct stat));
  memset(cli->file_type,'\0',20);
  memset(cli->file_path,'\0',50);
  cli->errno_set=0;
  
  return OK;
}

int recv_data(int fd,struct connect_status *cli){
  int i=0;
  int recv_data=1;
  while(recv_data){
    i=recv(fd,(cli->buffer + cli->buffer_size),MAXSIZE-cli->buffer_size,0);
    if(i<0){
      if((errno==EAGAIN)||(errno==EWOULDBLOCK))
	{
	  recv_data=0;
	  break;
	}
      else{
	recv_data=0;
	return -1;
	break;
      }
    }
    else if(i==0)
      {
	
	recv_data=0;
	return -1;
	break;
      }
    else{
      cli->buffer_size +=i;    
    }
  }
  return 0;
 
}




int send_data(int fd,char * buf,int size){
  int i;
  i=send(fd,buf,size,MSG_NOSIGNAL);
  if(i<0){
    if(errno==EPIPE){
      return -10;
    }/*
    else if(errno==EAGAIN){
      printf("send buffer full!!\n");
      sleep(5);
      }*/
    else{
      return -10;
    }
  }
  return i;
}

int send_process(int fd,struct connect_status *cli){
  int i;
  sprintf(cli->send,"%s\r\n",cli->send);
  i=send_data(fd,cli->send,strlen(cli->send));
  if(i==-10){
    cli->phase=CLOSE_PHASE;
    return -1;
  }
  if(cli->errno_set==1){
    return -1;
  }
  if(0!=(cli->file_stat.st_size)){
    int file_fd=open(cli->file_path,O_RDONLY);
    if(file_fd==-1){
      printf("open file error:%s",cli->file_path);
      return -1;
    }

    i=sendfile(fd,file_fd,NULL,cli->file_stat.st_size);
    if(i==-1){
      printf("send file ERROR ##%d\n",errno);
      close(file_fd);
      return -1;
    }
    close(file_fd);

/*
    char *file_buf;
    file_buf=mmap(NULL,cli->file_stat.st_size,PROT_READ,MAP_PRIVATE,file_fd,0);
    if(file_buf==MAP_FAILED){
      printf("mmap ERROR:::%d\n",errno);
      close(file_fd);
      return -1;
    }
    i=send_data(fd,file_buf,cli->file_stat.st_size);
    munmap(file_buf,cli->file_stat.st_size);
    close(file_fd);
    if(i==-10){
      cli->phase=CLOSE_PHASE;
      return -1;
      }

*/

  }
  i=send_data(fd,"\r\n\r\n\0",strlen("\r\n\r\n\0"));
  if(i==-10){
    cli->phase=CLOSE_PHASE;
    return -1;
  }
  return OK;
  
}

int process(int fd,struct connect_status *cli){
  int i;
  int out=0;
  char word;
  if(cli->rpc.read_set==0){
  cli->rpc.method=cli->buffer;
  }
  for(i=cli->rpc.read_set+1;i<cli->buffer_size;i++){
    word=cli->buffer[i];
    switch(word){
    case ' ':
      cli->rpc.mark=0;
      cli->rpc.blank++;
      switch(cli->rpc.blank){
      case 1:
	cli->buffer[i]='\0';
	cli->rpc.path=&(cli->buffer[i+1]);break;
      case 2:
	cli->buffer[i]='\0';
	cli->rpc.version=&(cli->buffer[i+1]);break;
      }break;
      
    case '\r':
      if(cli->rpc.mark==0)
	{
	  cli->rpc.mark=1;
	}
      else
	{
	  cli->rpc.rpc_done=1;
	  out=1;
	}
      if(cli->rpc.blank==2){
	cli->buffer[i]='\0';
      }break;
      
    case '\n':
      if(cli->rpc.blank==2){
	cli->buffer[i]='\0';
	}break;

    default:
      cli->rpc.mark=0;
      break;
    }
    
    if(out==1)
      break;
  }

  cli->rpc.read_set = i;
  if(cli->rpc.rpc_done==1)
    {
      return 0;
    }
  else
    {
      return -1;
    }
#if DEBUG_T
  printf("Method:%s\nPath:%s\nVersion:%s\n",cli->rpc.method,cli->rpc.path,cli->rpc.version);
#endif

}


int method_process(int fd,struct connect_status *cli){
  int i;
#if DEBUG_T
  printf("method_process\n");
#endif
  if((i=strcmp(cli->rpc.method,"GET"))==0){
    cli->rpc.method_i=METHOD_GET;
  }
  else if((i=strcmp(cli->rpc.method,"HEAD"))==0){
    cli->rpc.method_i=METHOD_HEAD;
  }
  else if((i=strcmp(cli->rpc.method,"POST"))==0){
    cli->rpc.method_i=METHOD_POST;
  }
  else{
    return -1;
    printf("#%d,METHOD_PROCESS ERROR",fd);
  }
  return 0;
}


int get_process(int fd,struct connect_status *cli){
#if DEBUG_T
  printf("get_process()\n");
#endif
  sprintf(cli->file_path,"%s%s\0",HOME_DIR,cli->rpc.path);
  int i;
  i=stat(cli->file_path,&(cli->file_stat));
  if(i!=0){
    printf("#%s doesn't exist!!\n",cli->file_path);
    ERR_PAGE_SET(404,cli);
 }
  else{
    if(S_ISDIR(cli->file_stat.st_mode)){
      sprintf(cli->file_path,"%s%s\0",cli->file_path,DEFAULT_PAGE);
      i=stat(cli->file_path,&(cli->file_stat));
      if(i!=0){
	printf("SECOND:%s doesn't exist\n\n",cli->file_path);
	ERR_PAGE_SET(404,cli);
      }
      cli->status=200;
    }
    else if(S_ISREG(cli->file_stat.st_mode)){
      cli->status=200;
      return OK;}
  }
}


int ERR_PAGE_SET(int ERR,struct connect_status *cli){
  if(404==ERR){
    sprintf(cli->file_path,"%s",ERR_404_PAGE);
    cli->status=404;
    stat(cli->file_path,&(cli->file_stat));
  }
}

int file_type_process(int fd,struct connect_status *cli){
  char *word;
  int i;
  int j;
  printf("file_type_process:%s\n",cli->file_path);
  word=strrchr(cli->file_path,'.');
  if(word==NULL){
    printf("word is NULL\n");
    sprintf(cli->file_type,"%s","text/plain");
  }else{
    word++;
    printf("word:%s\n",word);
    if((i=strcasecmp(word,"jpg"))==0){
      sprintf(cli->file_type,"%s","image/jpeg");
    }
    else if((i=strcasecmp(word,"htm"))==0){
	sprintf(cli->file_type,"%s","text/html");
      }
    else if((i=strcasecmp(word,"html"))==0){
      sprintf(cli->file_type,"%s","text/html");
    }
    else if((i=strcasecmp(word,"ico"))==0){
      sprintf(cli->file_type,"%s","image/x-icon");
    }
     else if((i=strcasecmp(word,"js"))==0){
      sprintf(cli->file_type,"%s","application/x-javascript");
      cli->errno_set=1;
      }
    else if((i=strcasecmp(word,"gif"))==0){
      sprintf(cli->file_type,"%s","image/gif");
    }
    else if((i=strcasecmp(word,"css"))==0){
      sprintf(cli->file_type,"%s","text/css");
      }
    else{
      sprintf(cli->file_type,"%s","text/plain");
    }
  }
#if DEBUG_T
  printf("file_type:%s\n",cli->file_type);
#endif
}

int pack_process(int fd,struct connect_status *cli){
  sprintf(cli->send,"%s",HTTP_SERVER_VERSION);
  switch(cli->status){
  case 200:
    sprintf(cli->send,"%s %s\r\n",cli->send,"200 OK");
    break;
  case 404:
    sprintf(cli->send,"%s %s\r\n",cli->send,"404 Page Not Found");
    break;
  }
  sprintf(cli->send,"%s%s%s\r\n",cli->send,"Content-Type: ",cli->file_type);
  sprintf(cli->send,"%s%s%d\r\n",cli->send,"Cotent-Length: ",(int)cli->file_stat.st_size);

#if DEBUG_T
  printf("\n\n###\n%s\n\n",cli->send);
#endif
}
