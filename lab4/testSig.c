#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int counter = 0;

void handler(int signo){
    if (signo == SIGUSR1){
        printf("catched SIGUSR1\n");
        counter++;
        printf(" Counter = %d\n",counter);
    }else if(signo == SIGUSR2){
        printf("catched SIGUSR2\n");
    }
}

void intHandler(int signo){
    if (signo == SIGINT){
        "Get a SIGINT\nExit from program\n";
        _exit(0);
    }
}

int main(){
    signal(SIGINT,intHandler);
    pid_t pid = fork();
    if (pid == 0){
        sigset_t sigs;
        sigemptyset(&sigs);
        sigaddset(&sigs,SIGKILL);
        sigaddset(&sigs,SIGSTOP);
        sigaddset(&sigs,SIGUSR2);
        sigprocmask(SIG_BLOCK,&sigs,NULL);
        signal(SIGUSR1,handler);
        signal(SIGUSR2,handler);
        while(1);
    }else{    
        sleep(1);
        kill(pid,SIGKILL);
        while(1){
            printf("Seng a SIGUSR1\n");
            kill(pid,SIGUSR1);
            kill(pid,SIGUSR2);
            sleep(3);
        }
    }
    return 0;
}