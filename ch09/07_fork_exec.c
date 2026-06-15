#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    printf("父进程即将fork+exec\n");
    pid_t pid = fork();
    if(pid == 0)
    {
        printf("子进程即将exec\n");
        execl("/bin/ls", "ls", "-l", NULL);
        perror("execl");          // exec 成功的话,这行永远不执行
        _exit(127);
    }
    else
    {
        wait(NULL);
        printf("父进程: 子进程执行完毕\n");
        
    }
    return 0;
}