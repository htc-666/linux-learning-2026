#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    int fd;
    fd = open("text.file.txt", O_RDWR | O_TRUNC | O_CREAT, 0744);
    if(-1 == fd)
    {
        perror("open Error");
    }

    pid_t pid = fork();
    if(pid == 0)
    {
        write(fd, "1234", 4);
        printf("子进程写完，偏移%ld\n", (long)lseek(fd, 0, SEEK_CUR));
        _exit(0);
    }
    else{
        sleep(1);
        write(fd, "AABB", 4);
        printf("父进程写完，偏移%ld\n", (long)lseek(fd, 0, SEEK_CUR));
        exit(0);
    }
    return 0;
}
