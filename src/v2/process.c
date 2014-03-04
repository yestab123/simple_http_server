int connect_init(int fd,struct connect_status *cli){
  cli->fd=fd;
  cli->phase=CONNECT_INIT_PHASE;
  cli->status=404;
  cli->rpc.way=NULL;
  cli->rpc.path=NULL;
  cli->rpc.version=NULL;
  memset(cli->buffer,'\0',500);
  cli->buffer_size=0;
  cli->send=NULL;
  send_size=0;
  send_done;
  return OK;
}

int recv_data(int fd,struct connect_status *cli){
  int i=0;
  do{
  i=recv(fd,(cli->buffer + cli->buffer_size),(MAXSIZE - cli->buffer),0);
  if(-1==i){
    return -1;}
  cli->buffer_size += i;
  }while(i!=0);
  return OK;
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
  cli->rpc.method=cli->buffer;
  for(i=0;i<cli->buffer_size;i++){
    mark=cli->buffer[i];
    switch(mark){
    case " ":blank++;
      switch(blank){
      case 1:cli->buffer[i]='\0';
	cli->rpc.path=&(cli->buffer[i+1]);break;
      case 2:cli->buffer[i]='\0';
	cli->rpc.version=&(cli->buffer[i+1]);break;
      }break;
    case '\r':if(blank==2){
	cli->buffer[i]='\0';}
    case '\n':if(blank==2){
	cli->buffer[i]='\0';
	out=1;}
    }
    if(out==1)
      break;
  }

}


int method_process(int fd,struct connect_status *cli){
  int i;
  if((i=strcmp(cli->rpc.method,"GET"))==0){
    cli->rpc.method_i=METHOD_GET;
    get_process(fd,cli);
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
  sprintf(path,"%s%s\0",HOME_DIR,path);
