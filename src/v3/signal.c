void signal_handler(int signal)
{
}


void signal_add(int signal){
  struct sigaction sa;
  memset(&sa,'\0',sizeof(sa));
  sa.sa_handler=signal_handler;
  sa.sa_flags|=SA_RESTART;
  sigfillset(&sa.sa_mask);
  int i=sigaction(signal,&sa,NULL);
  if(i<0)
    {
      log_write("signal add error",__FILE__,__LINE__,LOG_ERROR);
    }
}
