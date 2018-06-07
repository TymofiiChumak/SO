#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
int main(int argc, char** argv){
    int size = (int)strtol(argv[1],NULL,10);
    if (errno != 0) return EXIT_FAILURE;
    double *array = calloc(sizeof(double),size);
    return 0;
}