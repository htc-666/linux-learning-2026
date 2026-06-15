#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


static void cleanup(void *arg)
{
    printf("%s\n", (char*)arg);
}


static void* worker(void* arg)
{
    printf("新线程start\n");
    pthread_cleanup_push(cleanup, "资源1");
    pthread_cleanup_push(cleanup,"资源2");
    pthread_cleanup_push(cleanup,"资源3");

    pthread_exit((void*)0);

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
}


int main(void)
{
    pthread_t tid;

    pthread_create(&tid, NULL,worker, NULL);

    pthread_join(tid, NULL);

    return 0;
}