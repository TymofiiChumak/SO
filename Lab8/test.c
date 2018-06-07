#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *sum(void *arg){
    int *a = arg;
    int *res = malloc(sizeof(int));
    res[0] = a[0] + a[1];
    printf("%d + %d = %d\n",a[0],a[1],res[0]);
    return res;
}

int main(){
    pthread_t threads[5];
    for (int i = 0; i < 5; i++){
        int *args = malloc(2*sizeof(int));
        args[0] = i;
        args[1] = 5-i;
        pthread_create(&threads[i],NULL,&sum,args);
    }
    for (int i = 0; i < 5; i++){
        void *res;
        pthread_join(threads[i],&res);
        int *val = res;
        printf("#%d returned %d\n",i,*val);
    }
    return 0;
}