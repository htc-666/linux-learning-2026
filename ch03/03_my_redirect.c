#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    int fd1, fd2;
    fd1 = open("out.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    fd2 = dup2(fd1,1);
    close(fd1);
    printf("hello world\n");
    printf("Line 2\n");
    return 0;
}