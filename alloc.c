#include <stdio.h>
#include <stdlib.h>
void printArray(float array[], int size){
    for (int i = 0 ; i < size ; i ++){
        printf("%f ",array[i]);
    }
    printf("\n");
}

void all(int n){
    float *a1 = malloc(n*sizeof(float));
    float *a2 = calloc(0,n*sizeof(float));
    printArray(a1,n);
    printArray(a2,n);
}


int main(){
    all(10);
    return(0);
}


