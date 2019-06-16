#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <assert.h>
#include <netdb.h>

#define BUF_SIZE    1024

/*
编译 gcc tcp-select-server.c -o tcp-select-server
运行 ./tcp-select-server 50018
*/
void print_sockaddr(struct sockaddr_in addr)
{
    // 保存点分十进制的地址
    char ip_address[INET_ADDRSTRLEN];
    int port;

    inet_ntop(AF_INET, &addr.sin_addr, ip_address, sizeof(ip_address));
    port = ntohs(addr.sin_port);
    printf("(%s:%d)\n", ip_address, port);
}

int main(int argc, char *argv[])
{
    int sfd = -1;
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;

    struct sockaddr_in client_addr;
    struct sockaddr_in peer_addr;
    fd_set read_fds;
    fd_set work_fds;

    struct timeval tout;
    int peer_addrlen;
    int client_addrlen;
    int optval;
    int max_sockfd;

    char recv_buf[BUF_SIZE];
    int port = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        /* 允许IPv4 或者 IPv6 */
    hints.ai_socktype = SOCK_STREAM;    /* TCP */
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if ((setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                        &optval, sizeof (optval))) != 0)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != 0)
            continue;

        if (listen (sfd, 5) != 0)
            continue;

        /* 成功 */
        break;
    }
    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);

    FD_ZERO(&read_fds);
    FD_SET(sfd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    max_sockfd = sfd;

    while (1) {
        tout.tv_sec = 2;
        tout.tv_usec = 0;

        work_fds = read_fds;
        int ret = select (max_sockfd + 1, &work_fds, NULL, NULL, &tout);

        if (ret == 0) {
            continue;
        }
        if (ret == -1) {
            continue;
        }

        for (int i = 0; i < max_sockfd + 1; i++) {
            if (!FD_ISSET(i, &work_fds)) {
                continue;
            }

            int fd = i;
            if (fd == sfd) {
                client_addrlen = sizeof(client_addr);
                int cfd = accept(sfd, (struct sockaddr *)&client_addr,
                                 (socklen_t *)&client_addrlen);
                printf("accept fd:%d ", cfd);
                print_sockaddr(client_addr);

                if (cfd < 0) {
                    printf("accept cfd < 0!");
                    continue;
                }
                FD_SET(cfd, &read_fds);
                if (cfd > max_sockfd) {
                    max_sockfd = cfd;
                }
            } else {
                ssize_t num_read = recv(fd, recv_buf, sizeof(recv_buf), 0);
                if (num_read <= 0) {
                    printf ("client has left fd:%d\n", fd);
                    close(fd);
                    FD_CLR(fd, &read_fds);
                    continue;
                }

                recv_buf[num_read] = '\0';
                printf("receive %zd bytes: \"%s\" from fd:%d", num_read, recv_buf, fd);
                peer_addrlen = sizeof(peer_addr);
                getpeername(fd, (struct sockaddr *)&peer_addr, (socklen_t *)&peer_addrlen);
                print_sockaddr(peer_addr);

                send(fd, recv_buf, (size_t)num_read, 0);
            }
        }
    }

    return EXIT_SUCCESS;
}

