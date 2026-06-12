#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    if(fork() == 0)
    {
        printf("子进程即将死亡,pid=%d\n", getpid());
        _exit(0);
    }
    printf("父进程不回收\n");
    for(; ;)
    {
        sleep(1);
    }
    return 0;
}