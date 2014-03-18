#define LOG_DEBUG 1
#define LOG_MESSAGE 2
#define LOG_ERROR 3
#define LOG_STATUS 4

#define LOG_MAXSIZE 10000
#define DUMP_LOG_FILE "dump.log"
#define STATUS_LOG_FILE "status.log"

#define LOG_BUF_MAXSIZE 150

int log_msgid;

struct log_buf{
  char buf[LOG_BUF_MAXSIZE];
  int level;
  int size;
};
