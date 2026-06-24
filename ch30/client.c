#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SERVER_PORT 8888 //服务器的端口号
#define SERVER_IP "127.0.0.1"   //服务器的 IP 地址

int main(void)
{
    struct sockaddr_in client_addr = {0};
    int ret =0;
    int sockfd;
    char buf[512];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd <0)
    {
        perror("socket error");
        exit(-1);
    }

    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &client_addr.sin_addr);//IP 地址
    client_addr.sin_port = htons(SERVER_PORT);

    ret = connect(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if(ret <0)
    {
        perror("connect error");
        close(sockfd);
        exit(-1);
    }

    printf("连接成功\n");

    for(; ;)
    {
        memset(buf, 0, sizeof(buf));
        printf("please enter string\n");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {   // ⭐读到EOF/出错就退出
    break;
}
        ret = send(sockfd, buf, strlen(buf), 0);
        if(ret <0)
        {
            perror("ret error");
            close(sockfd);
            exit(-1);
        }
        if(0 == strncmp(buf, "exit", 4))
        break;
    }
    close(sockfd);
    return 0;
}