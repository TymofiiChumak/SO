//#define _POSIX_C_SOURCE 199309L
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#define MAX_CLIENT_NUMBER 1024
#define PATH "/home/tymofii"


struct request{
    pid_t pid;
    int index;    
};

struct clients_queue{
    int size;
    int begin;
    int end;
    struct request clients[MAX_CLIENT_NUMBER];
};


char* get_time(){
    struct timespec time_buffer;
    clock_gettime(CLOCK_MONOTONIC,&time_buffer);
    char *line;
    line = malloc(sizeof(char)*64);
    sprintf(line,"Time: %lld.%.09ld",time_buffer.tv_sec,time_buffer.tv_nsec);
    return line;
}
