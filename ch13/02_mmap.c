#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>


int main(int argc,char *argv[])
{
    int srcfd, dstfd;
    void * srcaddr;
    void * dstaddr;
    struct stat sbuf;
    if(3 != argc)
    {
        fprintf(stderr, "usage: %s <srcfile> <dstfile>\n", argv[0]);
        exit(-1);
    }
     srcfd = open(argv[1], O_RDONLY);
    if(-1 == srcfd)
    {
        perror("open error");
        exit(-1);
    }
    dstfd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664);
    if(-1 == dstfd)
    {
        perror("open error");
        exit(-1);
    }


    fstat(srcfd, &sbuf);
    ftruncate(dstfd, sbuf.st_size);

    srcaddr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, srcfd, 0);
    if(srcaddr == MAP_FAILED)
    {
        perror("error");
        exit(-1);
    }
    dstaddr = mmap(NULL, sbuf.st_size, PROT_WRITE, MAP_SHARED, dstfd, 0);
    if(MAP_FAILED == dstaddr)
    {
        perror("error");
        exit(-1);
    }

    memcpy(dstaddr, srcaddr, sbuf.st_size);

    return 0;
}