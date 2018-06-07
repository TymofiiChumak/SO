#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int x = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *fun(void *args){
    int num = ((int*)args)[0];
    while(1){
        pthread_mutex_lock(&mutex);
        x++;
        printf("in thread #%d, x: %d\n",num,x);
        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char** argv){
    int thread_num = atoi(argv[1]);
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