#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd ,n;
    char buffer[100];
    fd = open("test_file", O_RDONLY);
    if(fd == -1)
    {
        perror("error");
        return -1;
    }
    pread(fd, buffer, 100, 1024);
    n = lseek(fd, 0, SEEK_CUR);
    printf("%d\n", n);
    close(fd);
    return 0;
}