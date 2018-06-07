#include <stdio.h>
#include <stdlib.h>
#include <time.h>

float randf(){
    return (float)rand()/(float)(RAND_MAX);
}

int main(int argc, char **argv){
    int size = atoi(argv[1]);
    srand(time(NULL));
    float **buffer = malloc(size * sizeof(float*));
    float sum = 0.0;
    for (int i = 0; i < size; i++){
        buffer[i] = malloc(size * sizeof(float));
        for (int j = 0; j < size; j++){
            buffer[i][j] = randf();
            sum += buffer[i][j];
        }
    }
    FILE *dest = fopen("filter.txt","w");
    fprintf(dest,"%d\n",size);
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            fprintf(dest,"%f ",buffer[i][j] / sum);
        }
        fprintf(dest,"\n");
    }
    fclose(dest);

}