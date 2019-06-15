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
    fd_set read_fds;
    fd_set work_fds;

    struct timeval tout;
    int peer_addrlen;
    int client_addrlen;
    int optval;
    int max_sockfd;

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
                printf("accept fd %d ", cfd);
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
                ssize_t len = recv(fd, recv_buf, sizeof(recv_buf), 0);
                if (len <= 0) {
                    printf ("client fd %d has left\n", fd);
                    close(fd);
                    FD_CLR(fd, &read_fds);
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

