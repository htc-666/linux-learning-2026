#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler(int sig, siginfo_t *info, void *ctx)
{
    printf("收到 sig=%d, 数据=%d, 来自 PID=%d\n",
    sig, info->si_value.sival_int, info->si_pid);
}

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGRTMIN, &sa, NULL);
    
    pid_t pid = fork();
    if(pid == 0)
    {
        for(;;)
        pause();
    }
    else
    {
        sleep(1);
        for(int i =0; i <5; i++)
        {
            union sigval val = { .sival_int = i * 100 };
            sigqueue(pid, SIGRTMIN, val);
            puts("父:发了一个");
        }
        sleep(2);
        kill(pid, SIGTERM); 
    }
    return 0;
}