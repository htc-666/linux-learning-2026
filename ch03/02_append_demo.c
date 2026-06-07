#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd;
    char buffer[1];
    char buf[4];
    fd = open("test_file", O_RDWR | O_APPEND);
    if(fd == -1)
    {
        perror("Error:");
        return -1;
    }
    memset(buffer,0X55, sizeof(buffer));
    for(int i =0; i < 4;i++)
    {
        write(fd, buffer, sizeof(buffer));
    }
    lseek(fd, -4, SEEK_END);
    read(fd, buf, sizeof(buf));
    printf("0x%x 0x%x 0x%x 0x%x\n", buf[0], buf[1], buf[2], buf[3]);
    close(fd);
    return 0;
}