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
    if (argc < 2) {
        printf("Too few args");
        return EXIT_FAILURE;
    }
    FILE *source = fopen(argv[1],"r");
    if (!source){
        perror("File opening failed");
        return EXIT_FAILURE;
    }
    char ***buffer = calloc(sizeof(char**),LINES_NUM);
    int size_of_line[LINES_NUM];
    int number_of_lines = parse_file(buffer, size_of_line,  source);
    pid_t *pids = calloc(sizeof(pid_t),number_of_lines);
    char path[1024];
    getcwd(path, sizeof(path));
    char new_path[1024];
    sprintf(new_path,"PATH=%s", path);
    char *env[] = {new_path,NULL};
    for (int i = 0; i < number_of_lines ; i++){
        pid_t pid = fork();
        if (pid == 0){
            if (size_of_line[i] = 0){
                execve(buffer[i][0],NULL,env);
            }else {
                execve(buffer[i][0], buffer[i] , env);
            }
            _exit(0);
        }else{
            int status;
            waitpid(pid,&status,0);
            if (status != 0){
                printf("Error at %s",buffer[i][0]);
            }
        }
    }
    return 0;
}