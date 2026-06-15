#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

static void* worker(void* arg)
{
    int n = *((int*)arg);
    usleep(1000);
    printf("接收到的参数是%d\n", n);
    return NULL;
}

static int num[5] = {0,1,2,3,4};
int main(void)
{
    pthread_t   tid[5];


    printf("===== 错误版:传 &i =====\n");
    for(int i =0; i <5;i++)
    {
        pthread_create(&tid[i],NULL,worker,&i);
    }
    for(int i =0; i <5;i++)
    {
        pthread_join(tid[i], NULL);
    }
    printf("===== 正确版:传 num[i] =====\n");
    for(int i =0; i <5;i++)
    {
        pthread_create(&tid[i],NULL, worker, &num[i]);
    }
    for(int i =0; i <5;i++)
    {
        pthread_join(tid[i], NULL);
    }
    return 0;
}