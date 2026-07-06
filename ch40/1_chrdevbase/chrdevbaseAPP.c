#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("error usage\n");
        return -1;
    }

    int ret =0, fd;
    char readbuf[100];
    char writebuf[100] = "usr data!";
    char *name = argv[1];
    fd = open("name", O_RDWR);
    if(fd < 0)
    {
        printf("open error\n");
        return -1;
    }
    if(atoi(argv[2]) == 1)
    {
        ret = read(fd, readbuf, 50);
        if(ret < 0)
        {
            printf("read error\n");
            return -1;
        }
        printf("read success, data is:%s\n", readbuf);
    }

    if(atoi(argv[2]) == 2)
    {
        ret = write(fd, writebuf, sizeof(writebuf));
        if(ret < 0)
        {
            printf("write fail\n");
            return -1;
        }
        printf("write success\n");
    }
    ret = close(fd);
    if(ret <0)
    {
        perror("error");
        return -1;
    }
    return 0;
}
