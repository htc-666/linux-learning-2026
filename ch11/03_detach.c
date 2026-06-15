#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

static void*  worker(void* arg)
{
    pthread_detach(pthread_self());
    printf("我已分离\n");
    return NULL;
}


int main()
{
    pthread_t tid;
    int ret, ret1;
    void* a;
    ret = pthread_create(&tid, NULL, worker, NULL);
    if(ret)
    {
        printf("错误是：%s",strerror(ret));
        exit(-1);
    }
    sleep(2);
    ret1 = pthread_join(tid, &a);
    if(ret1)
    {
        printf("失败,原因是%s\n",strerror(ret1));
    }
    return 0;
}