#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void){
    int status;
    pid_t pid = fork();
    pid_t ret;
    if(pid == 0)
    {
        printf("子进程即将退出,pid是%d\n", getpid());
        abort();
    }
    else{
        printf("现在是父进程,pid=%d\n",getpid());
        ret = wait(&status);
        if(ret == -1)
        {
            perror("error");
        }
        else{
            printf("成功回收\n");
        }
        if(WIFEXITED(status))
        {
            printf("成功回收，返回码是%d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
        printf("子进程被信号 %d 杀死\n", WTERMSIG(status));
    }
    }
    return 0;
}