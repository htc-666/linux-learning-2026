#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

static void* worker(void* arg)
{
    int n = *((int *)arg);
    printf("子线程开始,线程id=%lu, 收到数据%d\n", pthread_self(),n);
    sleep(1);
    return (void*)(long)(n*n);
}

int main(void)
{
    pthread_t  t;
    int ret;
    void* a;
    int arg = 7;

    ret = pthread_create(&t, NULL,worker  ,&arg);
    if(ret)
    {
        perror("creat fail");
        exit(-1);
    }
    printf("主线程=%lu\n",pthread_self());
    pthread_join(t,&a);
    printf("子线程返回%ld\n", (long)a);
    return 0;
}