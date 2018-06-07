#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(){
    char prog[3][4] = {"./a", "./b", "./c"};
    pid_t *pids = calloc(sizeof(pid_t),3);
    for (int i = 0; i < 3 ; i++){
        pid_t pid = fork();
        if (pid == 0){
            execl(&prog[1],NULL);
            _exit(0);
        }else{
            pids[i] = pid;
        }
    }
    for (int i  = 0 ; i < 3 ; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        printf("Command %s has completed successfully by PID=%d\n", prog[i], pids[i]);
    }
}