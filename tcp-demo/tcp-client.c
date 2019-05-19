
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUF 512

int main(int argc, char *argv[])
{
    int sockfd;
    int len;
    struct sockaddr_in server_addr;
    char buf[MAXBUF + 1];
    fflush(stdin);

    fd_set rfds;
    struct timeval tv;
    int retval;
    int	maxfd = -1;

    if (argc != 3) {
        printf("usage %s <ip address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("already connected to server %s\n", argv[1]);

    while(1)
    {
        // 初始化rfds为空
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);			// 将标准输入的描述符0加入到集合rfds中
        FD_SET(sockfd, &rfds);		// 将newfd加入到集合rfds中

        maxfd = sockfd + 1;
        tv.tv_sec = 1;//阻塞等待时间为1s
        tv.tv_usec = 0;

        retval = select(maxfd, &rfds, NULL, NULL, &tv);//多路复用，同时监测描述符0和newfd

        // select函数执行出错
        if (retval == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if(retval == 0) {
            //select函数执行超时
            continue;
        } else {
            if(FD_ISSET(0, &rfds)) {
                bzero(buf, sizeof(buf));    //清空buf
                fgets(buf, sizeof(buf)-1, stdin);//从终端接收输入

                if (!strncasecmp(buf, "quit", 4)) {
                    printf("quit!\n");
                    break;
                }

                len = (int)send(sockfd, buf, strlen(buf)-1, 0);//向客户端发送消息
                if (len > 0) {
                    printf ("send successful,%d byte send!\n", len);
                } else {
                    printf("message '%s' send failure !\n", buf);
                    printf("errno code is %d, errno message is '%s'\n", errno, strerror(errno));
                    break;
                }
            }

            if (FD_ISSET(sockfd, &rfds)) {
                bzero(buf, sizeof(buf));
                len = (int)recv(sockfd, buf, sizeof(buf)-1, 0);//从客户端接收消息
                if (len > 0 ) {
                    printf("message recv successful : '%s', %d Byte recv\n", buf, len);
                } else if(len < 0) {
                    printf("recv failure !\nerrno code is %d, errno message is '%s'\n", errno, strerror(errno));
                    break;
                } else {
                    printf("the other one close, quit\n");
                    break;
                }
            }
        }
    }

    close(sockfd);
    printf("i quited!\n");
    return 0;
}


