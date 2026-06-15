#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

int main(void)
{
    pid_t pid = fork();
    if(pid >0)
    {
        exit(0);
    }
    else if(pid <0)
    {
        perror("error");
        exit(-1);
    }
    else{
        if(setsid()<0)
        {
            perror(("error"));
            exit(-1);
        }
        chdir("/");
        umask(0);

        close(0);
    close(1);
    close(2);

        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
        signal(SIGCHLD, SIG_IGN);

        for (;;) {
        int fd = open("/tmp/mydaemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd >= 0) {
            time_t t = time(NULL);
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "daemon alive: %s", ctime(&t));
            write(fd, buf, n);
            close(fd);
        }
        sleep(2);
        }
    }
    return 0;
}

