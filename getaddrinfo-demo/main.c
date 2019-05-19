
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 500

void print_sockaddr(struct sockaddr_in addr)
{
    // 保存点分十进制的地址
    char ip_address[INET_ADDRSTRLEN];
    int port;

    inet_ntop(AF_INET, &addr.sin_addr, ip_address, sizeof(ip_address));
    port = ntohs(addr.sin_port);
    // printf("connected client address = %s:%d\n",
    //                          inet_ntoa(connected_addr.sin_addr),
    //                          ntohs(connected_addr.sin_port));
    printf("ip address = %s:%d\n", ip_address, port);
}

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd = -1, s;

    // 分别表示监听的地址
    // struct sockaddr_in listendAddr;
    // 连接的本地地址，连接的对端地址
    struct sockaddr_in connected_addr, peer_addr;
    int connected_addrlen, peer_addrlen;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;           /* Any protocol */

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    printf("Connect %s:%s fd:%d\n", argv[1], argv[2], sfd);
    freeaddrinfo(result);

    // 获取connfd 表示的连接上的本地地址
    connected_addrlen = sizeof(connected_addr);
    getsockname(sfd, (struct sockaddr *)&connected_addr, (socklen_t *)&connected_addrlen);
    printf("connected client\n");
    print_sockaddr(connected_addr);

    // 获取connfd表示的连接上的对端地址
    peer_addrlen = sizeof(peer_addr);
    getpeername(sfd, (struct sockaddr *)&peer_addr, (socklen_t *)&peer_addrlen);
    printf("connected peer\n");
    print_sockaddr(peer_addr);

    /*
    int type;
    int type_len = sizeof(type);
    if (getsockopt(sfd, SOL_SOCKET, SO_TYPE,
                   (void *) &type, &type_len ) != 0 ) {
        if (type == SOCK_STREAM) {
            printf("socket stream\n");
        } else {
            printf("other\n");
        }
    } else {
        printf("failed\n");
    }
    */

    exit(EXIT_SUCCESS);
}



