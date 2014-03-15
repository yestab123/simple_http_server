

void * thread_process(void *arg){

}



void thread_init(int num){
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

