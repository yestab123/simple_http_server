#include <stdio.h>
#include <stdlib.h>
#include <unitd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

int
__creat_listen(char *ip, int port) {
    struct sockaddr_in ser;
    int sock, i;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    ser.sin_port = htons(port);
    ser.sin_family = AF_INET;
    ser.sin_addr.s_addr = inet_addr(ip);

    i = bind(sock, (struct sockaddr*)&ser, sizeof(ser));
    if (i < 0) {
        return i;
    }

    listen(sock, 10);

    return sock;
}

int
bb_creat_listen(char *ip, int port) {
    int i;

    i = __creat_listen(ip, port);
    if (i < 0) {
        return -1;
    }

    /* Socket Option Processing */
    bb_set_nonblock(i);

}
