#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(){
    key_t sem_key = ftok("/home/tymofii",1);
    int semset = semget(sem_key,1, 0666 | IPC_CREAT | IPC_EXCL);
    semctl(semset,0,SETVAL, 0);
    int val = semctl(semset,0,GETVAL);
    printf("Value: %d\n",val);
    struct sembuf buffer;
    buffer.sem_flg = 0;
    buffer.sem_op = 1;
    buffer.sem_num = 0;
    semop(semset,&buffer,1);
    val = semctl(semset,0,GETVAL);
    printf("Value: %d\n",val);
    semctl(semset,0,IPC_RMID);
    exit(EXIT_SUCCESS);
}