#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    printf("fork之前,pid是%d\n", getpid());
    pid_t pid = fork();
    printf("fork 之后 (PID=%d, fork返回=%d)\n", getpid(), pid);
    if(pid == 0)
    {
        printf("我是子进程\n");
        _exit(0);
    }
    else
    {
        printf("我是父进程\n");
        exit(0);
    }
    return 0;
}