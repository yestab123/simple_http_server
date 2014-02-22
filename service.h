static int p_to_p[2];
static pthread_mutex_t p_lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t clean_lock=PTHREAD_MUTEX_INITIALIZER;
static int fd_clean[2];
#define PATH_MAX 300
char DEFAULT_PATH[]={"/Project/web"};
char DEFAULT_FILE[]={"/index.html"};
char HTTP_VERSION[]={"HTTP/1.1"};
char SERVER_VERSION[]={"Wink-Chow Mini Web/0.1"};
char ERR_404[]={"/Project/web/404.html"};
char END_MESSAGE[]={"\n\r\n"};
