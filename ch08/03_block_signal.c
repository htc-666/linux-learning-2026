#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void my_handler(int sig)
{
    printf("handler跑了,sig = %d", sig);
}

int main(void)
{
    sigset_t sig_t;
    sigemptyset(&sig_t);
    sigaddset(&sig_t, SIGINT);
    struct sigaction sa = {0};
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigprocmask(SIG_BLOCK, &sig_t, NULL);
    puts("屏蔽了， 马上自发sigint\n");
    raise(SIGINT);

    printf("睡眠3s\n");
    sleep(3);

    printf("解锁后立刻执行\n");
    sigprocmask(SIG_UNBLOCK, &sig_t, NULL);
    puts("done");
    return 0;

}