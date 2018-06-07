#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/times.h>
#include <stdlib.h>
#include <fcntl.h>
#include <zconf.h>
#include <wait.h>
#include <sys/resource.h>

const int LINES_NUM = 100;
const int WORDS_NUM = 30;
const int WORDS_SIZE = 30;


int parse_file(char ***buffer, int *size_of_line, FILE *source){
    int size_of_word[LINES_NUM][WORDS_NUM];
    int i = 0;
    char c;
    size_of_line[0] = 0;
    size_of_word[0][0] = 0;
    buffer[0] = calloc(sizeof(char*), WORDS_NUM);
    buffer[0][0] = calloc(sizeof(char), WORDS_SIZE);
    char prev = (char)10;
    while ((c = fgetc(source)) != EOF) {
        if ((int)c == 10){
            if (prev == c) continue;
            size_of_line[i]++;
            buffer[i][size_of_line[i]] = NULL;
            i++;
            size_of_line[i] = 0;
            size_of_word[i][0] = 0;
            buffer[i] = calloc(sizeof(char*), WORDS_NUM);
            buffer[i][0] = calloc(sizeof(char), WORDS_SIZE);
        }else if ((int)c == 32){
            if (prev == c || prev == (char)10) continue;
            size_of_line[i]++;
            size_of_word[i][size_of_line[i]] = 0;
            buffer[i][size_of_line[i]] = calloc(sizeof(char), WORDS_SIZE);
        }else{
            buffer[i][size_of_line[i]][size_of_word[i][size_of_line[i]]] = c;
            size_of_word[i][size_of_line[i]]++;
        }
        prev = c;
    }
    return i;
}


int main(int argc, char **argv){
    if (argc < 3) {
        printf("Too few args");
        return EXIT_FAILURE;
    }
    FILE *source = fopen(argv[1],"r");
    if (!source){
        perror("File opening failed");
        return EXIT_FAILURE;
    }
    int time_limit = (int)strtol(argv[2],NULL,10);
    if (errno != 0){
        printf("Wrong time limit: %s\n",argv[4]);
        return EXIT_FAILURE;
    }    
    int memory_limit = (int)strtol(argv[3],NULL,10);
    if (errno != 0){
        printf("Wrong memory limit: %s\n",argv[4]);
        return EXIT_FAILURE;
    }
    memory_limit *= 1024;
    char ***buffer = calloc(sizeof(char**),LINES_NUM);
    int size_of_line[LINES_NUM];
    int number_of_lines = parse_file(buffer, size_of_line,  source);
    pid_t *pids = calloc(sizeof(pid_t),number_of_lines);
    char path[1024];
    getcwd(path, sizeof(path));
    char new_path[1024];
    sprintf(new_path,"PATH=%s", path);
    char *env[] = {new_path,NULL};
    struct rlimit time_lim, data_lim;
    time_lim.rlim_max=time_limit;
    time_lim.rlim_cur=time_limit;
    data_lim.rlim_max=memory_limit;
    data_lim.rlim_cur=memory_limit;
    struct rusage begin;
    getrusage(RUSAGE_SELF,&begin);
    for (int i = 0; i < number_of_lines ; i++){
        pid_t pid = fork();
        if (pid == 0){
            setrlimit(RLIMIT_CPU,&time_lim);
            setrlimit(RLIMIT_DATA,&data_lim);
            if (size_of_line[i] = 0){
                execve(buffer[i][0],NULL,env);
            }else {
                execve(buffer[i][0], buffer[i] , env);
            }
            _exit(0);
        }else{
            int status;
            waitpid(pid,&status,0);
            struct rusage end;
            getrusage(RUSAGE_CHILDREN,&end);
            printf("Process %d exited with status %d\n",pid,status);
            if (i == 0){
                printf("    System time: %lld.%06lld", 
                    end.ru_stime.tv_sec , 
                    end.ru_stime.tv_usec);
                printf("    User time: %lld.%06lld\n", 
                    end.ru_utime.tv_sec ,
                    end.ru_utime.tv_usec );
            } else {
                printf("    System time: %lld.%06lld", 
                    end.ru_stime.tv_sec - begin.ru_stime.tv_sec, 
                    end.ru_stime.tv_usec - begin.ru_stime.tv_usec);
                printf("    User time: %lld.%06lld\n", 
                    end.ru_utime.tv_sec - begin.ru_utime.tv_sec,
                    end.ru_utime.tv_usec - begin.ru_utime.tv_usec);  
            }  
            printf("Data size: %ld\n",end.ru_maxrss);
            begin = end;
            if (status != 0){
                printf("Error at %s Status %d",buffer[i][0],status);
            }
            printf("\n\n");
        }
    }
    return 0;
}