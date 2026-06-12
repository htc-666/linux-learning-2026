#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void  my_handler(int sig)
{
    printf("收到信号%d\n", sig);
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = my_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,&sa, NULL);
    while(1)
    {
         printf("running...\n");
        sleep(1);
    }
    return 0;
}



