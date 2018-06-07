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
    int n = strtol(argv[1],NULL,10);
    srand(time(NULL));
    
    char **args = malloc(4 * sizeof(char*));
    args[0] = "./client";
    args[2] = NULL;
    char *env[] = {NULL};

    for (int i = 0 ; i < n ; i++){
        char path[64];
        sprintf(path,"list%d.txt",i);
        args[1] = path;
        pid_t pid = fork();
        if (pid == 0){
            if (execvpe(args[0], args,env) != 0){
                perror(strerror(errno));
            }
        }
    }
    for (int i = 0 ; i < n ; i++){
        printf("End of #%d\n",i);
        wait(NULL);
    }

    return(0);
}