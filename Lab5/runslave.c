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


int main(int argc, char ** argv){
    if (argc < 2) perror("No args");
    char *path = argv[1];
    int n = strtol(argv[2],NULL,10);
    srand(time(NULL));
    char **args = malloc(4 * sizeof(char*));
    args[0] = "./slave";
    args[1] = path;
    args[2] = malloc(20 * sizeof(char));
    args[3] = NULL;
    char *env[] = {NULL};

    for (int i = 0 ; i < n ; i++){
        pid_t pid = fork();
        if (pid == 0){
            sprintf(args[2],"%d",rand()%10 + 1);
            if (execvpe(args[0], args,env) != 0){
                        printf("%s\n",strerror(errno));
                }
            exit(0);
        }
    }
    for (int i = 0 ; i < n ; i++){
        wait(NULL);
    }

    return(0);
}