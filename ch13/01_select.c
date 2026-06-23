#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>


int main(void)
{
    int ret=0, flag, s;
    char buf[100];
    fd_set  fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);

    flag = fcntl(0, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(0, F_SETFL, flag);
    for(int i =0; i <5;i++)
    {
        struct timeval tv = {5, 0};
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        ret = select(1, &fds, NULL,NULL,&tv);

    if(ret < 0)
    {
        perror("error");
        exit(-1);
    }
    else if (ret == 0)
    {
        printf("time out\n");
    }
    if(FD_ISSET(0, &fds))
    {
        s = read(0,buf, sizeof(buf));
        if(s>0)
        {
            printf("读取了%d字节\n", s);
        }
    }
    }
    return 0;
}