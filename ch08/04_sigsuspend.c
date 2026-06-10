#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t got_signal = 0;

void my_handler(int sig)
{
    got_signal = 1;
     puts("→ handler 触发");
}

int main(void)
{
    sigset_t new, old, wait;
    sigemptyset(&old);
    sigemptyset(&wait);
    sigemptyset(&new);
    sigaddset(&new, SIGUSR1);

    struct sigaction sa = {0};
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask);

    sigprocmask(SIG_BLOCK, &new, &old);
    sigaction(SIGUSR1, &sa, NULL);
    printf("PID = %d\n", getpid());
    for(int i =0; i <3; i++)
    {
        puts("主循环干活中(SIGUSR1 被屏蔽)");
        sleep(2);

        puts("调 sigsuspend,等 SIGUSR1...");
        sigsuspend(&wait);
         puts("被唤醒,继续干");
        got_signal = 0;
    }

    sigprocmask(SIG_UNBLOCK, &old, NULL);
}