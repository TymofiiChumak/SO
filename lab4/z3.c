#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>

volatile sig_atomic_t recived_from_parent = 0;
volatile sig_atomic_t recived_from_child = 0;
volatile sig_atomic_t send_to_child = 0;
volatile sig_atomic_t delivered = 0;

pid_t pid;
pid_t parent_pid;



void int_handler(int signo){
    if (signo == SIGINT){
        printf("\nReceived SIGINT. Exit from program \n");
        kill(pid, SIGUSR2);
        _exit(0);
    }
}

void child_handler(int signo){
    printf("Received from parent #%d\n", recived_from_parent);
    if (signo == SIGUSR1){
        recived_from_parent++;             
        kill(parent_pid,SIGUSR1);
    }else if (signo == SIGUSR2){
        kill(parent_pid,SIGUSR2);
        printf("\n  Recived from parent %d SIGUSR1 \n",recived_from_parent);
        _exit(0);
    }
}

void parent_handler(int signo){
    if (signo == SIGUSR1){
        printf("Received from child #%d\n", recived_from_child);    
        recived_from_child++;
        delivered = 1;
    }if (signo == SIGUSR2){
        printf("Reciving from child SIGUSR2\n");  
        printf("\n  Recived from child %d SIGUSR1 \n",recived_from_child);
        _exit(0);  
    }
}


int main(int argc, char **argv){
    int number = strtol(argv[1],NULL,10);
    int type = strtol(argv[2],NULL,10);
    parent_pid = getpid();
    pid = fork();
    if (pid == 0){
        printf("getpid %d\n",getpid());
        signal(SIGUSR1,child_handler);
        signal(SIGUSR2,child_handler);

        sigset_t *signals;
        sigfillset(signals);
        sigdelset(signals,SIGUSR1);
        sigdelset(signals,SIGUSR2);
        sigprocmask(SIG_BLOCK,signals,NULL);
        
        while(1);
    }else{
        sleep(1);
        printf("fork %d\n",pid);
        signal(SIGINT,int_handler);
        signal(SIGUSR1,parent_handler);
        signal(SIGUSR2,parent_handler);
        for (int i = 0; i < number ; i++){
            printf("\nSending to child #%d\n", send_to_child);
            switch (type){
                case 1: 
                    kill(pid,SIGUSR1);
                    break;
                case 2:
                    delivered = 0;
                    kill(pid,SIGUSR1);
                    while (delivered != 1);
                    break;
                case 3:
                    kill(pid,SIGRTMIN + 4);
                    break;
            }
            send_to_child++;            
        }
        switch (type){
            case 1:
                kill(pid,SIGUSR2);
                break;
            case 2:
                delivered = 0;
                kill(pid,SIGUSR2);
                while (delivered != 1);
                break;
            case 3:
                kill(pid,SIGRTMIN +9);
        }
        waitpid(pid,NULL,0);
    } 
    return(0);

}