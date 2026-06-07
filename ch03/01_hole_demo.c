#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{


int fd;
char buffer[1024];
fd = open("test_file", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if(fd == -1)
{
    perror("error:");
    return(-1);
}
lseek(fd, 4096, SEEK_SET);
memset(buffer, 0Xff, sizeof(buffer));
for(int i =0 ; i < 4;i++)
{
    write(fd, buffer, sizeof(buffer));
}
close(fd);
return 0;
}