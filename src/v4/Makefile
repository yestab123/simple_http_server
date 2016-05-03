server:server.o epoll.o share.o http.o process.o signal.o
	gcc   -o   server server.o epoll.o share.o http.o process.o signal.o -lpthread
server.o:server.c config.h
	gcc -c server.c
epoll.o:epoll.c config.h
	gcc -c epoll.c
share.o:share.c config.h
	gcc -c share.c
http.o:http.c http.h config.h
	gcc -c http.c
process.o:process.c process.h config.h
	gcc -c process.c

signal.o:signal.c config.h
	gcc -c signal.c

clean:
	rm server.o epoll.o share.o http.o process.o signal.c


