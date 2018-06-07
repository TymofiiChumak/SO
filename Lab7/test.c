#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>




int main(){
    
    pid_t pid = fork();
    if  (pid == 0){
        printf("Begin child\n");                
        sem_t *sem = sem_open("/sem",0);
        sem_wait(sem);
        int fd = shm_open("/test",O_RDWR,0);
        char *addr = mmap(NULL,64,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
        printf("[%s]\n",addr);
        close(fd);
        exit(EXIT_SUCCESS);
    }else{
        printf("Begin parent\n");
        sem_t *sem = sem_open("/sem",O_CREAT | O_EXCL,0666,1);
        sem_wait(sem);
        sleep(1);
        int fd = shm_open("/test",O_CREAT | O_EXCL | O_RDWR,0666);
        ftruncate(fd,64);
        char *addr = mmap(NULL,64,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
        char *buffer = "alamakota";
        memcpy(addr,buffer,15);
        close(fd);
        sem_post(sem);
        sleep(2);
        waitpid(pid,NULL,0);
        shm_unlink("/test");
        sem_unlink("/sem");
    }
    return 0;
}