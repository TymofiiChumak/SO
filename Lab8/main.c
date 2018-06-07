#define min(a,b) ((a > b) ? b : a)
#define max(a,b) ((a < b) ? b : a)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>


int **image;
int width;
int height;
float **filter;
int width_f,height_f;
int **dest_image;
int size;

struct timespec begin_time;
struct timespec end_time;

void start_timer(){
    clock_gettime(CLOCK_MONOTONIC,&begin_time);
}

void end_timer(FILE *time_raport){
    clock_gettime(CLOCK_MONOTONIC,&end_time);
    long long delta_sec = end_time.tv_sec - begin_time.tv_sec;
    long long delta_nsec = end_time.tv_nsec - begin_time.tv_nsec;
    if (delta_nsec < 0){
        delta_sec += 1;
        delta_nsec += 1000000000;
    }
    fprintf(time_raport,"Time: %lld.%.09ld\n",delta_sec,delta_nsec);
}

int **parse_pgm(FILE *source, int *w, int *h){
    char *text_buffer = malloc(1024);
    fscanf(source,"%[^\n]\n",text_buffer);
    fscanf(source,"%[^\n]\n",text_buffer);
    int width,height;
    fscanf(source,"%d",&width);      
    fscanf(source,"%d",&height);
    
    *w = width;
    *h = height;
    int max_point;
    fscanf(source,"%d",&max_point);
    if (max_point != 255){
        perror("not valid maxium gray value\n");
        exit(EXIT_FAILURE);
    }
    int **buffer = malloc(width * sizeof(int*));
    for (int i = 0; i < width; i++){
        buffer[i] = malloc(height * sizeof(int));
        for (int j = 0; j < height; j++){
            fscanf(source,"%d",&buffer[i][j]);
        }
    }
    return buffer;
}

float **parse_filter(FILE *source, int *w, int *h){
    int width,height;
    fscanf(source,"%d",&width);
    height = width;
    *w = width;
    *h = height;
    float **buffer = malloc(width * sizeof(float*));
    for (int i = 0; i < width; i++){
        buffer[i] = malloc(height * sizeof(float));
        for (int j = 0; j < height; j++){
            fscanf(source,"%f",&buffer[i][j]);
        }
    }
    return buffer;
}

void write_image(FILE *destination, int **image, int w, int h){
    fprintf(destination,"P2\n");
    fprintf(destination,"%d %d\n",w,h);
    fprintf(destination,"255\n");
    for (int i = 0; i < w; i++){
        for (int j = 0; j < h; j++){
            fprintf(destination,"%d ",image[i][j]);
        }
        fprintf(destination,"\n");
    }
}


int filter_pixel(int x, int y){
    float res = 0.0;
    for (int i = 0; i < width_f; i++){
        for (int j = 0; j < height_f; j++){
            int tmpx = x - size + i;
            int tmpy = y - size + j;
            if (tmpx >= 0 && tmpx < width && tmpy >= 0 && tmpy < height){
                res += image[tmpx][tmpy] * filter[i][j];
            }
        }
    }
    return round(res);
}

void *thread_filter(void *arg){
    int begin = ((int*)arg)[0];
    int end = ((int*)arg)[1];
    for (int i = 0; i < width; i++){
        for (int j = begin; j < end; j++){
            dest_image[i][j] =filter_pixel(i,j);
        }
    }
}



int main(int argc, char** argv){
    if (argc < 4){
        perror("Too few arguments\n");
        exit(EXIT_FAILURE);
    }
    int thread_num = atoi(argv[1]);
    FILE *source_image = fopen(argv[2],"r");
    if (source_image == NULL){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    FILE *source_filter = fopen(argv[3],"r");
    if (source_filter == NULL){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    FILE *destination = fopen(argv[4],"w");
    if (destination == NULL){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    FILE *time_raport = fopen("Times.txt","a");
    image = parse_pgm(source_image,&width,&height);
    filter = parse_filter(source_filter,&width_f,&height_f);
    size = ceil(width_f /2);
    dest_image = malloc(width * sizeof(int *));
    for (int i = 0; i < width; i++){
        dest_image[i] = malloc(height * sizeof(int));
    }
    fprintf(time_raport,"Image width: %d height: %d\n",width,height);
    fprintf(time_raport,"Filter size: %d\n",width_f);
    fprintf(time_raport,"Thread number: %d\n",thread_num);
    start_timer();
    pthread_t threads[thread_num];
    for (int i = 0; i < thread_num; i++){
        int *args = malloc(2*sizeof(int));
        args[0] = height * i / thread_num;
        args[1] = height * (i + 1) / thread_num;
        pthread_create(&threads[i],NULL,&thread_filter,args);
    }
    for (int i = 0; i < thread_num; i++){
        void *ret_val;
        pthread_join(threads[i],&ret_val);
    }
    end_timer(time_raport);
    
    write_image(destination,dest_image,width,height);
    fclose(source_filter);
    fclose(source_image);
    fclose(destination);
    fclose(time_raport);
    return 0;
}