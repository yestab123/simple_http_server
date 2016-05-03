#include "config.h"
#include "http.h"

static int method_process(struct http_request_s *p);
static int recv_process(int fd,struct http_request_s *p);
static int process_request(struct http_request_s *p);
static int http_content_init(http_request *p,char *path);
static int http_send_package_init(http_request *p);
static int http_send_phase(http_request *p);
int http_request_init(http_request *p,int fd,uint32_t event)
{
    p->fd=fd;
    p->now_event=event;
    p->time=0;
    p->phase=HTTP_INIT;

    

    //header_int INIT
    memset(p->header_in.buffer,'\0',BUFFER_MAXSIZE);
    memset(p->package_send.file_path,'\0',100);
    memset(p->package_send.buffer,'\0',1000);
    memset(p->header_out.content_type,'\0',20);
    p->header_in.max_size=BUFFER_MAXSIZE;
    p->header_in.start=0;
    p->header_in.end=0;
    p->header_in.now=0;
    p->header_in.ps=0;
    p->header_in.method.length=0;
    p->header_in.uri.length=0;
    p->header_in.version.length=0;
    p->recv_process=recv_process;
    p->method_process=method_process;
    p->process_request=process_request;
    p->send_process=http_send_package_init;
    p->send=http_send_phase;
    p->error_sum=0;
    //    printf("HTTP_REQUEST_INIT FINIST\n");
}

int http_process(http_request *p)
{
    //    printf("http_process\n");
    //printf("PHASE:%d\n",p->phase);
    /* switch(p->phase)
        {
        case HTTP_INIT:
        case HTTP_RECV:p->recv_process(p->fd,p);break;
        case HTTP_METHOD:
        case HTTP_URI:
        case HTTP_VERSION:p->method_process(p);break;
        case HTTP_RECV_DONE:
        case HTTP_PROCESS:p->process_request(p);break;
        case HTTP_PROCESS_DONE:p->send_process(p);break;
        case HTTP_SEND:p->send(p);break;
        default:break;
        }*/
    if(p->phase==HTTP_INIT||p->phase==HTTP_RECV)
        {
            p->recv_process(p->fd,p);
        }
    if(p->phase==HTTP_METHOD||p->phase==HTTP_URI||p->phase==HTTP_VERSION)
        {
            p->method_process(p);
        }
    if(p->phase==HTTP_RECV_DONE||p->phase==HTTP_PROCESS)
        {
            p->process_request(p);
        }
    if(p->phase==HTTP_PROCESS_DONE)
        {
            p->send_process(p);
        }
    if(p->phase==HTTP_SEND)
        {
            p->send(p);
        }
}

int method_process(struct http_request_s *p)
{
    http_in *q=&(p->header_in);
    char word;
    int i=0;
    char *point;
    if(q->now!=0)
        {
            point=&(q->buffer[q->now]);
            if(q->uri.name==NULL)
                {
                    for(i=q->now;i<q->end;i++)
                        {
                            word=(q->buffer[i]);
                            switch(word)
                                {
                                case ' ':
                                    q->uri.name=point;
                                    q->uri.length=i-(q->now);
                                    point=&(q->buffer[i+1]);
                                    q->now=i+1;
                                    q->buffer[i]='\0';
                                    goto METHOD_NEXT;
                                    break;
                                }
                        }
                }
        METHOD_NEXT:if(q->version.name==NULL)
                {
                    for(i=q->now;i<q->end;i++)
                        {
                            word=(q->buffer[i]);
                            switch(word)
                                {
                                case '\r':
                                    q->version.name=point;
                                    q->version.length=i-(q->now);
                                    q->now=i+2;
                                    break;
                                    
                                }
                        }
                }
            goto METHOD_END;
        }
    int blank=0;
    point=&(q->buffer[0]);
    for(i=0;i<q->end;i++)
        {
            word=q->buffer[i];
            switch(word)
                {
                case ' ':
                    if(blank==0)
                        {
                            blank++;
                            q->method.name=point;
                            q->method.length=i-(q->now);
                            point=&(q->buffer[i+1]);
                            q->now=i+1;
                            q->buffer[i]='\0';
                            break;
                        }
                    if(blank==1)
                        {
                            blank++;
                            q->uri.name=point;
                            q->uri.length=i-(q->now);
                            point=&(q->buffer[i+1]);
                            q->now=i+1;
                            q->buffer[i]='\0';
                        }
                    break;
                case '\r':
                   
                    q->version.name=point;
                    //printf("||||%d,%d\n",i,q->now);
                    q->version.length=i-(q->now);
                    q->now=i+2;
                    q->buffer[i]='\0';
                }
            
        }
METHOD_END:    if((q->method.name==NULL) || (q->uri.name==NULL) || (q->version.name==NULL))
        {
            p->phase=HTTP_RECV;
            return -1;
        }
    else
        {
            p->phase=HTTP_RECV_DONE;
            return 0;
        }
    
}

int recv_process(int fd,struct http_request_s *p)
{
    //    printf("recv_process\n");
    p->error_sum++;
    if(p->error_sum>=50)
        {
            p->phase=HTTP_CLOSE;
            return -1;
        }
    if((p->header_in.end) >= (p->header_in.max_size))
        {
            printf("%d:RECV out of BUFFER\n",fd);
            p->phase=HTTP_CLOSE;
            return -1;
        }
    int i;
    http_in *q=&(p->header_in);
    while(1)
        {
            i=recv(fd,q->buffer,(q->max_size - q->end),0);
            if(i<0)
                {
                    if(errno==EAGAIN||errno==EWOULDBLOCK)
                        {
                            break;
                        }
                    p->phase=HTTP_CLOSE;
                    break;
                }
            if(i==0)
                {
                    p->phase=HTTP_CLOSE;
                    break;
                }
            q->end+=i;
        }
    p->phase=HTTP_METHOD;
    return 0;
}

int process_request(struct http_request_s *p)
{
    //    printf("%s\n",p->header_in.buffer);
    http_in *q=&(p->header_in);
    if(q->method.length==3)
        {
            if(strncmp(q->method.name,"GET",3)==0)
                {
                    q->method_i=HTTP_GET_METHOD;
                }
        }
    else
        {
            p->phase=HTTP_CLOSE;
            return -1;
        }

    char file_path[50];
    char request_path[30];
    memset(request_path,'\0',30);
    strncpy(request_path,q->uri.name,q->uri.length);
    // printf("##METHOD:%d,%s##\n",q->method.length,q->method.name);
    //printf("##LENGTH:%d,%s##\n",q->uri.length,q->uri.name);
    //printf("##VERSION:%d,%s##\n",q->version.length,q->version.name);
    sprintf(file_path,"%s%s",DEFAULT_PATH,request_path);
    struct stat temp_stat;
    int i;
    i=stat(file_path,&temp_stat);
    // printf("0.PATH:%s\n",file_path);
    if(i!=0)
        {
            p->header_out.http_status=404;
            printf("1.stat.error:%s\n",file_path);
            p->phase=HTTP_PROCESS_DONE;
            return -1;
        }
    else
        {
            if(S_ISDIR(temp_stat.st_mode))
                {
                    sprintf(file_path,"%s/%s",file_path,DEFAULT_PAGE);
                    i=stat(file_path,&(temp_stat));
                    if(i!=0)
                        {
                            p->header_out.http_status=404;
                        }
                    else
                        p->header_out.http_status=200;
                }
            else
                p->header_out.http_status=200;
        }
    if(p->header_out.http_status==404)
        {
            p->phase=HTTP_PROCESS_DONE;
            return -1;
        }
    p->phase=HTTP_PROCESS;
    //    printf("##PATH:%s\n",file_path);
    strcpy(p->package_send.file_path,file_path);
    stat(file_path,&(p->header_out.file_stat));
    http_content_init(p,file_path);
    return 0;
}


int http_content_init(http_request *p,char *path)
{
      char *word;
  int i;
  int j;

  word=strrchr(path,'.');
  if(word==NULL){
    sprintf(p->header_out.content_type,"%s","text/plain");
  }else{
    word++;

    if((i=strcasecmp(word,"jpg"))==0){
      sprintf(p->header_out.content_type,"%s","image/jpeg");
    }
    else if((i=strcasecmp(word,"htm"))==0){
	sprintf(p->header_out.content_type,"%s","text/html");
      }
    else if((i=strcasecmp(word,"html"))==0){
      sprintf(p->header_out.content_type,"%s","text/html");
    }
    else if((i=strcasecmp(word,"ico"))==0){
      sprintf(p->header_out.content_type,"%s","image/x-icon");
    }
     else if((i=strcasecmp(word,"js"))==0){
      sprintf(p->header_out.content_type,"%s","application/x-javascript");
      }
    else if((i=strcasecmp(word,"gif"))==0){
      sprintf(p->header_out.content_type,"%s","image/gif");
    }
    else if((i=strcasecmp(word,"css"))==0){
      sprintf(p->header_out.content_type,"%s","text/css");
      }
    else{
      sprintf(p->header_out.content_type,"%s","text/plain");
    }
  }
  p->header_out.content_length=p->header_out.file_stat.st_size;
  p->phase=HTTP_PROCESS_DONE;
  return 0;
}

int http_error_page_init(http_request *p)
{
    http_package *q=&p->package_send;
    memset(q->buffer,'\0',500);
    q->sendfile=0;
    q->buffer_start=0;
    q->buffer_end=0;
    q->buffer_pos=0;
    q->sendfile_done=1;
    sprintf(q->buffer,"%s\r\n%s\r\n","HTTP/1.1 404 BAD","Content-type: text/plain");
    sprintf(q->buffer,"%s%s%d\r\n\r\n%s",q->buffer,"Content-length: ",strlen(ERROR_BUFFER),ERROR_BUFFER);
    q->buffer_end=strlen(q->buffer);
    return 0;
}

int http_send_package_init(http_request *p)
{
    if(p->header_out.http_status!=200)
        {
            http_error_page_init(p);
            p->phase=HTTP_SEND;
            return 0;
        }
    http_package *q=&p->package_send;
    q->sendfile=1;
    q->buffer_start=0;
    q->buffer_end=0;
    q->buffer_pos=0;
    q->sendfile_done=0;
    sprintf(q->buffer,"%s\r\n%s%s\r\n%s%d\r\n\r\n","HTTP/1.1 200 OK","Content-Type:",p->header_out.content_type,"Content-Length: ",p->header_out.content_length);
    q->buffer_end=strlen(q->buffer);
    p->phase=HTTP_SEND;
    return 0;
}

int http_send_phase(http_request *p)
{
    http_package *q=&p->package_send;
    //    printf("##||##send:%s\n",q->buffer);
    if(q->buffer_pos < q->buffer_end)
        {
            int i=send(p->fd,(q->buffer+q->buffer_pos),(q->buffer_end)-(q->buffer_pos),MSG_NOSIGNAL);
            if(i<0)
                {
                    if(errno==EAGAIN)
                        {
                            return 1;
                        }
                    p->phase=HTTP_CLOSE;
                }
            else
                {
                    q->buffer_pos+=i;
                }
        }
    if(q->buffer_pos>=q->buffer_end && q->sendfile==0)
        {
            p->phase=HTTP_SEND_DONE;
            return 0;
        }
    if((q->buffer_pos>=q->buffer_end)&&(q->sendfile==1))
       {
           if(q->sendfile_done==0)
               {
                   if(0!=p->header_out.file_stat.st_size)
                       {
                           p->header_out.file_fd=open(q->file_path,O_RDONLY);
                           if(p->header_out.file_fd==-1)
                               {
                                   p->phase=HTTP_SEND_DONE;
                                   printf("##OPENFILE_ERROR:%s\n",q->file_path);
                                   return -1;
                               }
                           int i=sendfile(p->fd,p->header_out.file_fd,NULL,p->header_out.file_stat.st_size);
                           if(i==-1)
                               {
                                   close(p->header_out.file_fd);
                                   printf("Sendfile,ERROR\n");
                                   p->phase=HTTP_SEND_DONE;
                                   return -1;
                               }
                           printf("%d.SENDFILE:FD:%d,%d\n",p->fd,p->header_out.file_fd,i);
                           close(p->header_out.file_fd);
                           p->phase=HTTP_SEND_DONE;
                           q->sendfile_done=1;
                           return 0;
                       }
                   else
                       {
                           q->sendfile_done=1;
                           p->phase=HTTP_SEND_DONE;
                           return 0;
                       }
               }
           else
               {
                   p->phase=HTTP_SEND_DONE;
                   return 0;
               }
       }
}
