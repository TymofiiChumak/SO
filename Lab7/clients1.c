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
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include "util1.c"

int server_memory;
sem_t *server_semaphore1;
sem_t *server_semaphore2;
struct clients_queue *clients;
sem_t *client_semaphore;
char *sem_name;



int set_elem(struct clients_queue *queue, struct request *buffer);


void handler(int signo){    
    sem_unlink(sem_name);
    printf("Received signal\n");
    exit(EXIT_SUCCESS);
}

void alarm_handler(int signo){
    printf("received alarm signal\n");
}


int main(int argc, char **argv){
    signal(SIGALRM,alarm_handler);
    if (argc < 3){
        perror("TOO FEW ARGS\n");
        exit(EXIT_FAILURE);
    }
    int client_number = strtol(argv[1],NULL,10);
    int handle_number = strtol(argv[2],NULL,10);
    int server_memory_size = sizeof(struct clients_queue) * MAX_CLIENT_NUMBER;
    if ((server_memory = shm_open("/servermem",O_RDWR,0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    if ((server_semaphore1 = sem_open("/serversem1",0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    if ((server_semaphore2 = sem_open("/serversem2",0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    if ((clients = mmap(NULL,sizeof(struct clients_queue),PROT_READ | PROT_WRITE,MAP_SHARED,server_memory,0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    for (int i = 0; i < client_number; i++){

        pid_t pid = fork();
        if (pid == 0){
            printf("Created client #%d pid: %d %s\n",i,getpid(),get_time());
            signal(SIGINT,handler);            
            sem_name = malloc(32*sizeof(char));
            sprintf(sem_name,"/clientsem%d",i);
            if ((client_semaphore = sem_open(sem_name,O_CREAT | O_EXCL,0666,0)) < 0){
                perror(strerror(errno));
                exit(EXIT_SUCCESS);
            }
            printf("%d\n",client_semaphore);
            struct request buffer;
            buffer.pid = getpid();
            buffer.index = i;
            for (int j = 0; j < handle_number; j++){
                int queue_size;
                if ((queue_size = set_elem(clients,&buffer)) == 0){
                    printf("Server wakes %s\n",get_time());
                }
                
                printf("Client %d wait #%d left %d %s\n",getpid(),j+1,queue_size,get_time());
                sem_wait(client_semaphore);
                printf("Client %d begin #%d %s\n",getpid(),j+1,get_time());
                sem_wait(client_semaphore);
                printf("Client %d end #%d %s\n",getpid(),j+1,get_time());
                sleep(2);
            }
            printf("End client #%d pid: %d %s\n",i+1,getpid(),get_time());
            sem_unlink(sem_name);
            exit(EXIT_SUCCESS);
        }
    }
    for (int i = 0; i < client_number; i++){
        wait(NULL);
    }
    return 0;
}


int set_elem(struct clients_queue *queue, struct request *buffer){
    sem_wait(server_semaphore2);
    int size = queue->size;
    if (queue->size >= MAX_CLIENT_NUMBER-1) return -1;
    queue->end++;
    if (queue->end == MAX_CLIENT_NUMBER) queue->end = 0;
    memcpy(&queue->clients[queue->end],buffer,sizeof(struct request));
    queue->size++;
    sem_post(server_semaphore1);
    sem_post(server_semaphore2);
    return size;
}