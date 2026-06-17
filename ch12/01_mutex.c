#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

static int g_count = 0;
static pthread_mutex_t mutex;

static void*  worker(void* arg)
{
    int  l_count;
    for(int i =0; i < 10000000;i++)
    {
        pthread_mutex_lock(&mutex);
        l_count = g_count;
        l_count++;
        g_count = l_count;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(void)
{
    pthread_t tid1, tid2;
    int ret1, ret2;
    pthread_mutex_init(&mutex,NULL);
    ret1 = pthread_create(&tid1, NULL, worker, NULL);
    if(ret1)
    {
        printf("error:%s\n", strerror(ret1));
        exit(-1);
    }
    ret2 = pthread_create(&tid2, NULL, worker, NULL);
    if(ret2)
    {
        printf("error:%s\n", strerror(ret2));
        exit(-1);
    }
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_mutex_destroy(&mutex);
    printf("g_count = %d\n", g_count);
    return 0;
}