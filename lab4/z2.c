#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <errno.h>

int confirmed;
int received = 0;
int from[1000];
int k;

void parent_handler(int signo, siginfo_t* info, void* vp){
    pid_t pid = info->si_pid;
    from[received] = pid;
    received++;
    printf("Received request from process: %d %d\n",pid,received);
    if (received == k){        
        for (int i = 0 ; i < k; i++){
            printf("Sent permission to process: %d\n",from[i]);
            kill(from[i],SIGUSR1);
        }
    }else if (received > k){
        printf("Sent permission to process: %d\n",pid);
        kill(pid,SIGUSR1);
    }
}

void rt_handler(int signo, siginfo_t* info, void* vp){
    printf("Recived Rt sygnal SIGRTMIN + %d from process: %d\n",signo - SIGRTMIN,info->si_pid);
}

void child_handler(int signo){
    if (signo == SIGUSR1){
        confirmed = 1;
    }
}



int main(int argc, char **argv){
    int n = strtol(argv[1],NULL,10);
    k = strtol(argv[2],NULL,10);

    struct sigaction sa1;
    sa1.sa_sigaction = parent_handler;
    sigemptyset(&sa1.sa_mask); 
    sa1.sa_flags = 0;
    sa1.sa_flags = sa1.sa_flags | SA_SIGINFO | SA_NODEFER;
    sigaction(SIGUSR1, &sa1, NULL);   

    struct sigaction sa2;
    sa2.sa_sigaction = rt_handler;
    sigemptyset(&sa2.sa_mask); 
    sa2.sa_flags = 0;
    sa2.sa_flags = sa2.sa_flags | SA_SIGINFO | SA_NODEFER;
    for (int i = 0 ; i < 32 ; i++){
        sigaction(SIGRTMIN + i, &sa2, NULL); 
    }
    struct sigaction sa;
    sa.sa_handler = child_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    pid_t parent = getpid();        
    for (int i = 0; i < n; i++){
        pid_t pid = fork();
        if (pid == 0){             
            printf("Created new process with PID: %d\n",getpid());
            sigaction(SIGUSR1, &sa, NULL); 
            srand(time(NULL)+i);
            int wait_time = (rand() % 10 +1);
            sleep(wait_time);
            pid_t pid = getpid();
            confirmed = 0;
            union sigval sv;
            sv.sival_int = 0;
            sv.sival_ptr = 0;
            sigqueue(parent, SIGUSR1,sv);
            while (confirmed != 1){
            }
            kill(parent,SIGRTMIN + (rand() % 32));
            _exit(wait_time);
        }
    }
    for(int i = 0 ; i < n; i++){
        int status;
        int pid;
        while ((pid = wait(&status)) == -1){}
        printf("Process %d ended with status: %d\n",pid,WEXITSTATUS(status));
    }
    return(0);
}

    