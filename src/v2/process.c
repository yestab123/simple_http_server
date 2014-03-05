int connect_init(int fd,struct connect_status *cli){
  cli->fd=fd;
  cli->phase=CONNECT_INIT_PHASE;
  cli->status=404;
  cli->rpc.method=NULL;
  cli->rpc.path=NULL;
  cli->rpc.version=NULL;
  memset(cli->buffer,'\0',MAXSIZE);
  cli->buffer_size=0;
  memset(cli->send,'\0',MAXSIZE);
  cli->send_size=0;
  cli->send_done=0;
  return OK;
}

int recv_data(int fd,struct connect_status *cli){
  int i=0;
  int recv_data=1;
  while(recv_data){
    i=recv(fd,(cli->buffer + cli->buffer_size),MAXSIZE-cli->buffer_size,0);
    // printf("recv_data loop\n");
    if(i<0){
      if((errno==EAGAIN)||(errno==EWOULDBLOCK))
	{
	  recv_data=0;
	  break;
	}
      else{
	close(fd);
	recv_data=0;
	break;
      }
    }
    else if(i==0)
      {
	close(fd);
	recv_data=0;
	break;
      }
    else{
      cli->buffer_size +=i;
      //printf("Get data #%d\n",i);
    }
  }
  //printf("recv_data() DONE\n");
}

int send_data(int fd,struct connect_status *cli){
  int i=0;
  while((cli->send_size - cli->send_done) != 0){
    i=send(fd,(cli->send + cli->send_done),(cli->send_size - cli->send_done),MSG_NOSIGNAL);
    if(-1 == i){
      if(errno==EPIPE){
	return CLOSE;
      }
      else
	return -1;
    }
    cli->send_done += i;
  }
  return OK;
}

int process(int fd,struct connect_status *cli){
  int i;
  int blank=0;
  int out=0;
  char mark;

  printf("process() init \n");

  cli->rpc.method=cli->buffer;
  for(i=0;i<cli->buffer_size;i++){
    mark=cli->buffer[i];
    switch(mark){
    case ' ':blank++;
      switch(blank){
      case 1:cli->buffer[i]='\0';
	cli->rpc.path=&(cli->buffer[i+1]);break;
      case 2:cli->buffer[i]='\0';
	cli->rpc.version=&(cli->buffer[i+1]);break;
      }break;
    case '\r':if(blank==2){
	cli->buffer[i]='\0';};break;
    case '\n':if(blank==2){
	cli->buffer[i]='\0';
	out=1;}break;
    }
    if(out==1)
      break;
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
    printf("#%d,METHOD_PROCESS ERROR",fd);
  }
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
