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

#define BUF_SIZE    1024

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

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct sockaddr_in peer_addr;
    int peer_addrlen;
    int client_addrlen;
    int optval, maxsockfd;

    char recv_buf[BUF_SIZE];
    int port = 0;

    if ((sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        port = atoi(argv[1]);
    } else {
        port = 5678;
    }

    printf("listen port %d, server_fd: %d  \n", port, sfd);
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

    memset(&client_addr, 0, sizeof(client_addr));
    memset(&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind (sfd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
        printf("bind failed\n");
    }

    if (listen (sfd, 10) < 0) {
        printf("listen failed");
    }

    fd_set readfds;
    fd_set workfds;
    maxsockfd = sfd;
    struct timeval tout;

    FD_ZERO(&readfds);
    FD_SET(sfd, &readfds);
    FD_SET(STDIN_FILENO, &readfds);

    while (1) {
        struct timeval tout;
        tout.tv_sec = 2;
        tout.tv_usec = 0;

        workfds = readfds;
        int ret = select (maxsockfd + 1, &workfds, NULL, NULL, &tout);

        if (ret == 0) {
            continue;
        }
        if (ret == -1) {
            continue;
        }

        for (int i = 0; i < maxsockfd + 1; i++) {
            if (!FD_ISSET(i, &workfds)) {
                continue;
            }

            int fd = i;
            if (fd == sfd) {
                client_addrlen = sizeof(client_addr);
                int cfd = accept(sfd, (struct sockaddr *)&client_addr,
                                 (socklen_t *)&client_addrlen);
                printf("accept fd %d ", cfd);
                print_sockaddr(client_addr);

                if (cfd < 0) {
                    printf("accept cfd < 0!");
                    continue;
                }
                FD_SET(cfd, &readfds);
                if (cfd > maxsockfd) {
                    maxsockfd = cfd;
                }
            } else {
                ssize_t len = recv(fd, recv_buf, sizeof(recv_buf), 0);
                if (len <= 0) {
                    printf ("client fd %d has left\n", fd);
                    close(fd);
                    FD_CLR(fd, &readfds);
                    continue;
                }
                // printf("the client_ip : %s\n", inet_ntoa(client_addr.sin_addr));
                recv_buf[len] = '\0';
                printf("%s ", recv_buf);
                peer_addrlen = sizeof(peer_addr);
                getpeername(fd, (struct sockaddr *)&peer_addr, (socklen_t *)&peer_addrlen);
                print_sockaddr(peer_addr);

                send(fd, recv_buf, (size_t)len, 0);
            }
        }
    }


    return EXIT_SUCCESS;
}

