#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void my_handler(int sig)
{
    printf("接收到信号%d\n", sig);
}

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGABRT, &sa, NULL);

    puts("即将abort");
    raise(SIGABRT);

    puts("失败");
    return 0;
}