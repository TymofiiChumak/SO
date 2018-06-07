#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


int semset;
void handler(int signo){
    int val = semctl(semset,0,GETVAL);
    printf("Final Value: %d\n",val);

    semctl(semset,0,IPC_RMID);  
    exit(EXIT_SUCCESS);  
}

int main(){
    key_t sem_key = ftok("/home/tymofii",1);
    semset = semget(sem_key,1, 0666 | IPC_CREAT | IPC_EXCL);
    semctl(semset,0,SETVAL, 0);
    int val = semctl(semset,0,GETVAL);
    printf("Value before fork: %d\n",val);

    signal(SIGINT,handler);
    pid_t pid = fork();

    if (pid == 0){
        while(1){
            sleep(1);
            struct sembuf buffer;
            buffer.sem_flg = 0;
            buffer.sem_op = -1;
            buffer.sem_num = 0;
            semop(semset,&buffer,1);
            val = semctl(semset,0,GETVAL);
            printf("Value at child: %d\n",val);
        }
    }else{
        while(1){
            struct sembuf buffer;
            buffer.sem_flg = 0;
            buffer.sem_op = 1;
            buffer.sem_num = 0;
            semop(semset,&buffer,1);
            val = semctl(semset,0,GETVAL);
            printf("Value at parent: %d\n",val);
            sleep(2);
        }
    }
}