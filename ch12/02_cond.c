#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

static int g_avail =0;
static pthread_mutex_t mutex;
static pthread_cond_t cond;

static void* worker(void* arg)
{
    for(int i=0; i <5;i++)
    {
    pthread_mutex_lock(&mutex);
    while(g_avail <=0)
    {
        pthread_cond_wait(&cond,&mutex);
    }
        g_avail--;
        printf("消费一个， 剩余%d\n",g_avail);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}



int main(void)
{
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex,NULL);

    int ret;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, worker, NULL);
    if(ret)
    {
        printf("error:%s",strerror(ret));
        exit(0);
    }

    for(int i =0; i <5; i++)
    {
        sleep(1);
        pthread_mutex_lock(&mutex);
        g_avail++;
        printf("生产一个，库存=%d\n", g_avail);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }
    pthread_join(tid, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
