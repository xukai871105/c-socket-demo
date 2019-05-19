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
#include "utlist.h"


#define BUF_SIZE    1024


typedef struct fd_el
{
    int fd;
    struct fd_el *next, *prev;
} fd_el_t;


int main(int argc, char *argv[])
{
    int sfd = -1;
    fd_el_t *e = NULL;
    fd_el_t *head = NULL;
    fd_el_t *tmp = NULL;

    struct sockaddr_in ser_addr;
    struct sockaddr_in client_addr;
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

    printf("listen port %d, fd = %d  \n", port, sfd);
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

    memset(&client_addr, 0, sizeof(client_addr));
    memset(&ser_addr, 0, sizeof (ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind (sfd, (struct sockaddr *) &ser_addr, sizeof (ser_addr) < 0)) {
        printf("bind failed\n");
    }

    if (listen (sfd, 10) < 0) {
        printf("listen failed");
    }

    fd_set fdset;
    maxsockfd = sfd;
    struct timeval tout;


    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(sfd, &fdset);

        struct timeval tout;
        tout.tv_sec = 5;
        tout.tv_usec = 0;

        LL_FOREACH(head, e) {
            if (e->fd != 0) {
                printf("%02d ", e->fd);
                FD_SET(e->fd, &fdset);
            }
        }
        printf("\n");

        int ret = select (maxsockfd + 1, &fdset, NULL, NULL, &tout);

        if (ret == 0) {
            continue;
        }
        if (ret == -1) {
            printf("Error select ...!\n");
            continue;
        }

        if (FD_ISSET(sfd, &fdset))
        {
            int cfd_len = sizeof (client_addr);
            int cfd = accept(sfd, (struct sockaddr *)&client_addr,
                             (socklen_t *)&cfd_len);
            printf("accept fd %d \n", cfd);

            if (cfd < 0) {
                printf("accept cfd <= 0!");
                continue;
            }

            fd_el_t *new_el = (fd_el_t*)malloc(sizeof(fd_el_t));
            new_el->fd = cfd;
            LL_PREPEND(head, new_el);

            if (cfd > maxsockfd) {
                maxsockfd = cfd;
            }
        }

        LL_FOREACH_SAFE(head, e, tmp) {
            if (FD_ISSET(e->fd, &fdset)) {
                ssize_t len = recv(e->fd, recv_buf, sizeof (recv_buf), 0);
                if (len <= 0) {
                    printf ("client fd %d has left\n", e->fd);
                    close(e->fd);
                    FD_CLR(e->fd, &fdset);
                    LL_DELETE(head, e);
                    free(e);
                    continue;
                }

                printf("the client_ip : %s\n", inet_ntoa(client_addr.sin_addr));
                recv_buf[len] = '\0';
                printf("%s\n", recv_buf);
                send(e->fd, recv_buf, (size_t)len, 0);
            }
        }

    }

    LL_FOREACH(head, e) {
        close(e->fd);
    }

    return EXIT_SUCCESS;
}

