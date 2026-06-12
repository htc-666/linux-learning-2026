#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int global = 100;

int main(void)
{
    int local = 200;
    pid_t pid = fork();

    if(pid == 0)
    {
        global = 200;
        local = 400;
        printf("global = %d, local = %d\n", global, local);
        _exit(0);
    }
    else{
        sleep(1);
        printf("global = %d, local = %d\n", global, local);
        exit(0);
    }
    return 0;
}