
void signal_handler(int sig){
  int save_errno=errno;
  int msg=sig;
  send(fd_clean[1],(char*)&msg,1,0);
  errno=save_errno;
}


int add_signal(int sig){
  struct sigaction sa;
  memset(&sa,'\0',sizeof(sa));
  sa.sa_handler=SIG_IGN;
  sa.sa_flags|=SA_RESTART;
  sigemptyset(&sa.sa_mask);
  int i;
  i=sigaction(sig,&sa,NULL);
  if(i==0)
    printf("##signal set\n");
  if(i!=0)
    printf("signal add error\n");
}
