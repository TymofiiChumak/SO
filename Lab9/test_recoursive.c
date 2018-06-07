#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>


int x = 0;
pthread_mutex_t mutex;


void *fun(void *args){
    int num = ((int*)args)[0];
    while(1){
        pthread_mutex_lock(&mutex);
        pthread_mutex_lock(&mutex);
        x++;
        if (x > 1000) return 0;
        printf("in thread #%d, x: %d\n",num,x);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char** argv){
    int thread_num = atoi(argv[1]);
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex,&attr);
    pthread_t threads[thread_num];
    int numbers[thread_num];
    for (int i = 0; i < thread_num; i++){
        numbers[i] = i;
        pthread_create(&threads[i],NULL,&fun,&numbers[i]);
    }
    for (int i = 0; i < thread_num; i++){
        pthread_join(threads[i],NULL);
    }
    return 0;
}