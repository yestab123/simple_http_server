#define HTTP_BUFFER_SIZE 2000

struct rpc_t{
  char *method;
  char *path;
  char *version;
};

struct http_t{
  int fd;
  char buffer[HTTP_BUFFER_SIZE];
  int buffer_count;
  int process;
  int status;
  struct rpc_t rpc;
  char file_path[PATH_MAX];
  struct stat file_st;
  
};

int path_merge(struct http_t *http)
{
  sprintf(http->file_path,"%s%s\0",DEFAULT_PATH,http->rpc.path);
  int i;
  i=stat(http->file_path,&(http->file_st));
  if(i!=0){
    printf("#%s doesn't exit!!!\n",http->file_path);
    http->status=404;
    return -2;
  }
  if(S_ISDIR((http->file_st.st_mode)))
    {
      sprintf(http->file_path,"%s%s",http->file_path,DEFAULT_FILE);
      i=stat(http->file_path,&(http->file_st));
      if(i!=0)
	{
	  printf("##DIR:%s doesn't exit index.html\n",http->file_path);
	  http->status=404;
	  return -2;
	}
    }
  http->status=200;
  return 0;
}

int rpc_process(struct http_t *http)
{
  int blank=0;
  int i;
  http->rpc.method=http->buffer;
  for(i=0;i<(http->buffer_count);i++)
    {
      if(blank<2){
	if(http->buffer[i]==' '){
	  switch(blank){
	  case 0:http->buffer[i]='\0';http->rpc.path=&(http->buffer[i+1]);blank++;break;
	  case 1:http->buffer[i]='\0';http->rpc.version=&(http->buffer[i+1]);blank++;break;
	}
      }
      }
    }
}

int process_test(struct http_t http)
{
  pthread_t th;
  th=pthread_self();
  printf("PTHREAD:%lu  ",(unsigned long)th);
  printf("method:%s\n",http.rpc.method);
  printf("path:%s\n\n",http.file_path);
  //printf("version:%s\n",http.rpc.version);
}

int return_message_test(struct http_t http)
{
  char OK_BUFFER[]={"HTTP/1.1 200 OK\nServer: Wink-Chow Web_Service/0.1\nContent-Type:text/plain\nContent-length: 26\n\nHi!The Service is working!\n\r\n"};
  send(http.fd,OK_BUFFER,strlen(OK_BUFFER),0);
  return 1;
}

int send_message(int fd,void * send_buf)
{
  int i;
  i=send(fd,send_buf,strlen(send_buf),MSG_NOSIGNAL);
  return i;
}

int message_handle(struct http_t *http)
{
  char recv_pack[1000];
  sprintf(recv_pack,"%s ",HTTP_VERSION);
  switch(http->status){
  case 200:
    sprintf(recv_pack,"%s%s",recv_pack,"200 OK\n");break;
  case 404:
    sprintf(recv_pack,"%s%s",recv_pack,"404 Not Found\n");break;
  }
  sprintf(recv_pack,"%s%s%s\n",recv_pack,"Server: ",SERVER_VERSION);
  sprintf(recv_pack,"%s%s%d\n",recv_pack,"Conten-length: ",http->file_st.st_size);
  sprintf(recv_pack,"%s\n",recv_pack);

  int i;
  i=send_message(http->fd,recv_pack);
  if(-1==i)
    return -1;
  char *file_buf;
  int fd;
  fd=open(http->file_path,O_RDONLY);
  if(0!=(http->file_st.st_size)){
  file_buf=mmap(NULL,http->file_st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
  if(file_buf==MAP_FAILED){
    printf("mmap ERROR\n");
    return -1;
  }
  send_message(http->fd,file_buf);
  }

  send_message(http->fd,END_MESSAGE);
}

  


int process(int fd)
{
  struct http_t http;
  http.fd=fd;
  int i;
  i=recv(fd,http.buffer,HTTP_BUFFER_SIZE,0);
  if(i==0)
    {
      //  printf("|||(%d)||| closed\n",fd);
      return 5;
    }
  http.buffer_count=i;
  http.buffer[i]='\0';
  sprintf(http.file_path,"%s\0","/");
   rpc_process(&http);
   i=path_merge(&http);
   if(0==i)
     {
              message_handle(&http);
     }
   else if(i==(-2))
     {
       sprintf(http.file_path,"%s",ERR_404);
       i=stat(http.file_path,&(http.file_st));
       if(i!=0)
	 printf("#######ERROR STAT 404.html\n");
        message_handle(&http);
     }
       
   //return_message_test(http);
   process_test(http);
   
   return 0;
}
