#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>

pid_t child_pid;
volatile sig_atomic_t stoped;

void int_handler(int signo){
    if (signo == SIGINT){
        if (stoped == 0){
            printf("\nRecived signal SIGINT\n");
            kill(child_pid,SIGINT);
            _exit(0);
        }else{
            kill(child_pid,SIGINT);
            _exit(0);
        }
    }
}


void stop_handler(int signo){
    if (signo == SIGTSTP){
        if (stoped == 0){
            kill (child_pid,SIGKILL);
            printf("\nWait for: \n    CTRL+Z - continue\n    CTRL+C - end\n");
            stoped = 1;
        }else {          
            printf("\n");
            stoped = 0;
        }    
    }
}


int main(){
    stoped = 0;
    struct sigaction act;
    act.sa_handler = stop_handler;
    sigemptyset(&act.sa_mask); 
    act.sa_flags = 0; 
    sigaction(SIGTSTP, &act, NULL); 
    signal(SIGINT,int_handler);
    while (1){
        if (stoped == 0){
            child_pid = fork();
            if (child_pid == 0){
                if (execl("writeDate.sh",NULL) == -1){
                    printf("Can't open writeDate.sh\n");
                }
            }else{
                waitpid(child_pid,NULL,0);
            }
        }
    }
    return(0);

}
