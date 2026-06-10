#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>



void my_handler(int sig)
{
    printf("Alarm timeout\n");
    exit(0);
}

int main(void)
{
    struct sigaction   sa;
    sa.sa_flags =0;
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGALRM, &sa, NULL);
    alarm(5);
    for (;;) {
        puts("working...");
        sleep(1);
    }
}