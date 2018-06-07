#define _XOPEN_SOURCE 500
#define _GNU_SOURCE

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
#include <unistd.h>

const int LINES_NUM = 100;
const int WORDS_NUM = 30;
const int WORDS_SIZE = 30;
int file_end = 0;



int parse_line(char ***buffer, int *size_of_line, FILE *source){
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
            if (strlen(buffer[i][size_of_line[i]]) != 0){
                size_of_line[i]++;
                buffer[i][size_of_line[i]] = NULL;
            }else{
                buffer[i][size_of_line[i]] = NULL;
            }
            break;
        }else if (c == '|'){
            if (prev == c) continue;
            if (strlen(buffer[i][size_of_line[i]]) != 0){
                size_of_line[i]++;
                buffer[i][size_of_line[i]] = NULL;
            }else{
                buffer[i][size_of_line[i]] = NULL;
            }
            i++;
            size_of_line[i] = 0;
            size_of_word[i][0] = 0;
            
            buffer[i] = calloc(sizeof(char*), WORDS_NUM);
            buffer[i][0] = calloc(sizeof(char), WORDS_SIZE);
        }else if ((int)c == 32){
            if (prev == c || prev == (char)10) continue;
            if (strlen(buffer[i][size_of_line[i]]) != 0){
                size_of_line[i]++;
                buffer[i][size_of_line[i]] = NULL;
                size_of_word[i][size_of_line[i]] = 0;
                buffer[i][size_of_line[i]] = calloc(sizeof(char), WORDS_SIZE);
            }            
        }else{
            buffer[i][size_of_line[i]][size_of_word[i][size_of_line[i]]] = c;
            size_of_word[i][size_of_line[i]]++;
        }
        prev = c;
    }
    if (c == EOF){
        file_end = 1;
        if (strlen(buffer[i][size_of_line[i]]) != 0){
            size_of_line[i]++;
        }else{
            buffer[i][size_of_line[i]] = NULL;
        }
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
    while (file_end == 0){
        char ***buffer = calloc(sizeof(char**),LINES_NUM);
        int size_of_line[LINES_NUM];
        int number_of_lines = parse_line(buffer, size_of_line,  source);
        for (int i = 0; i <= number_of_lines; i++){
            int j = 0;
            while (buffer[i][j] != NULL){
                printf("|%s| ",buffer[i][j]);
                j++;
            }
            printf("\n");
        }
        if (buffer[0][0] == NULL) continue;
        pid_t *pids = calloc(sizeof(pid_t),number_of_lines);
        char path[1024];
        getcwd(path, sizeof(path));
        char new_path[1024];
        sprintf(new_path,"PATH=%s", path);
        char *env[] = {new_path,NULL};
        int pipes_fd[number_of_lines+1][2];
        for (int i = 0; i <= number_of_lines ; i++){
            pipe(pipes_fd[i]);
        }
        for (int i = 0; i <= number_of_lines ; i++){
            pid_t pid = fork();
            if (pid == 0){
                if (i != 0){
                    dup2(pipes_fd[i-1][0],0); //stdin
                    //close(pipes_fd[i-1][1]);                
                }
                if (i != number_of_lines){
                    dup2(pipes_fd[i][1],1); //stdout
                    //close(pipes_fd[i][0]);
                }
                if (size_of_line[i] = 0){
                    if (execvpe(buffer[i][0],NULL,env) != 0){
                        printf("%s\n",strerror(errno));
                        dup2(1,pipes_fd[i][1]);
                    }
                }else if (execvpe(buffer[i][0], buffer[i],env) != 0){
                        printf("%s\n",strerror(errno));
                        dup2(1,pipes_fd[i][1]);
                }
                close(pipes_fd[i-1][0]);
                close(pipes_fd[i][1]);
                _exit(0);
            }else{
                int status;
                waitpid(pid,&status,0);
                if (status != 0){
                    printf("Error at %s status: %d\n",buffer[i][0],status);
                }
            }
        }
    }
    fclose(source);
    return 0;
}