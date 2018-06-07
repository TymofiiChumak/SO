#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

int allow;
int k_pot;
int number;

void handler(int signo){
    if(signo == SIGUSR1){
        allow = 1;
    }
}

int recieved[1000];

void handle_syg(pid_t pid){
    printf("Otrzymano prosbe od potomka %d #%d\n", pid ,number);
        if(k_pot >= number ){
            if(k_pot == number ){
                for(int i = 0; i <= number+10; i++){
                    if (recieved[i]!= 0){
                        kill(recieved[i], SIGUSR1);
                        printf("Wyslano pozwolenie do %d\n", recieved[i]);
                    }
                } 
            }
        }else{
            kill(pid, SIGUSR1);
            printf("Wyslano pozwolenie do %d\n", pid);
        }
}

void handler1(int signo, siginfo_t *info, void *vp){
    if(signo == SIGUSR1){
        recieved[number] = info->si_pid;        
        number++;
        handle_syg(info->si_pid);
        
    }else if (signo == SIGINT){
        _exit(0);
    }
}

void handler2(int signo, siginfo_t *info, void *vp){
    printf("przyjeto sygnal SIGMIN + %d od %d\n", signo - SIGRTMIN, info->si_pid);
    
}



int main(int argc, char *argv[]){
    int n_pot = atoi (argv[1]);
    k_pot = atoi (argv[2]);
    number = 0;
    struct sigaction s1;
    s1.sa_sigaction = handler1;
    s1.sa_flags = 0;
    s1.sa_flags = s1.sa_flags | SA_SIGINFO | SA_NODEFER;
    sigemptyset(&s1.sa_mask);
    sigaction(SIGUSR1, &s1, NULL); 
    sigaction(SIGINT, &s1, NULL); 
    

    struct sigaction s3;
    s3.sa_sigaction = handler2;
    s3.sa_flags = SA_SIGINFO;
    sigemptyset(&s3.sa_mask);

    for(int i = 0; i < 32; i++){
        sigaction(SIGRTMIN+i, &s3, NULL);
        recieved[i] = 0;
    }   
    struct sigaction s2;
    s2.sa_handler = handler;
    s2.sa_flags = 0;
    sigemptyset(&s2.sa_mask);
    pid_t ppid = getpid();
    for(int i = 0; i < n_pot; i++){
        pid_t pid = fork();
        if(pid == 0){
            srand(time(NULL) * i); // 1523442118;
            int time_sleep  = rand()%10 + 1; 
            printf("stworzono nowy procces %d %d\n", getpid(),time_sleep);
            sigaction(SIGUSR1, &s2, NULL);
            sleep(time_sleep);
            allow = 0;
            union sigval sv;
            sv.sival_int = 0;
            sv.sival_ptr = 0;
            sigqueue(ppid, SIGUSR1,sv);
            printf("--sended %d--\n",i);
            while(allow != 1){
            }
            kill(ppid, SIGRTMIN + rand()%32);
            _exit(time_sleep);
        }
    }
    
    for(int i = 0; i < n_pot; i++){
        int status;
        pid_t pid;
        while ((pid = waitpid(-1,&status,0)) == -1){
        }
        printf("Skonczono process %d z statusem %d - %d\n", pid,WEXITSTATUS(status),i);
    }
    return 0;
}