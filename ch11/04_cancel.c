#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

static void* worker(void* arg)
{
    printf("子线程开始\n");
    for(;;)
    {
        pthread_testcancel();
    }
    return NULL;
}

int main(void)
{
    pthread_t tid;
    int ret;
    void* a;
    ret = pthread_create(&tid, NULL, worker, NULL);
    if(ret)
    {
        printf("错误：%s", strerror(ret));
        exit(-1);
    }

    pthread_cancel(tid);
    pthread_join(tid, &a);
    sleep(1);
    if(a == PTHREAD_CANCELED)
    {
        printf("子线程已被取消\n");
    }
    else{
        printf("子线程没被取消\n");
    }
    return 0;
}