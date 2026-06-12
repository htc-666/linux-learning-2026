#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void wait_child(int sig)
{
    while(waitpid(-1, NULL, WNOHANG)>0)
    {
        printf("子进程被消灭\n");
    }
}

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = wait_child;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);


    for(int i =0; i <5;i++)
    {
        if(fork() == 0)
        {
            printf("子进程%d被创建,pid = %d\n", i, getpid());
            sleep(2);
            exit(0);
        }
    }
    for(; ;)
    {
        sleep(1);
    }
    return 0;
}