#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT    8888


int main(void)
{
    struct sockaddr_in  socket_addr = {0};
    struct sockaddr_in  conn_addr = {0};
    int sockfd, connfd;
    int ret;
    socklen_t addrlen = sizeof(conn_addr);
    char buf[512];


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd <0)
    {
        perror("socket error");
        exit(-1);
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_addr.sin_port = htons(SERVER_PORT);

    ret = bind(sockfd, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
    if(ret <0)
    {
        perror("bind error");
        close(sockfd);
        exit(-1);
    }

    ret = listen(sockfd, 50);
    if(ret <0)
    {
        perror("listen error");
        close(sockfd);
        exit(-1);
    }

    connfd = accept(sockfd, (struct sockaddr*)&conn_addr, &addrlen);
    if(0 > connfd)
    {
        perror("accept error");
        close(sockfd);
        exit(-1);
    }
    printf("接收到客户端数据\n");
    for(; ;)
    {
        memset(buf, 0, sizeof(buf));
        ret = recv(connfd, buf, sizeof(buf), 0);
        if(ret <0)
        {
            perror("recv error");
            close(sockfd);
            exit(-1);
        }
        else if (ret == 0) {           // ⭐ 对方关闭连接
        printf("客户端断开\n");
        break;
        }
        if (strncmp(buf, "exit", 4) == 0) break;
    }
    close(sockfd);
    close(connfd);
    return 0;
    
}