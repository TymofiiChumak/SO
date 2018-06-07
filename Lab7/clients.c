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
#include "util.c"

int server_memory;
int server_semaphore;
struct clients_queue *clients;
int client_semaphore;



int set_elem(struct clients_queue *queue, struct request *buffer, int semaphore);


void handler(int signo){    
    semctl(client_semaphore,0,IPC_RMID);
    printf("Received signal\n");
    exit(EXIT_SUCCESS);
}


int main(int argc, char **argv){
    if (argc < 3){
        perror("TOO FEW ARGS\n");
        exit(EXIT_FAILURE);
    }
    int client_number = strtol(argv[1],NULL,10);
    int handle_number = strtol(argv[2],NULL,10);

    key_t server_memory_key = ftok(PATH,1);
    key_t server_semaphore_key = ftok(PATH,2);
    int server_memory_size = sizeof(struct clients_queue) * MAX_CLIENT_NUMBER;
    if ((server_memory = shmget(server_memory_key,server_memory_size,0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    if ((server_semaphore = semget(server_semaphore_key,1,0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    clients = (struct clients_queue *)shmat(server_memory,NULL,0);

    for (int i = 0; i < client_number; i++){

        pid_t pid = fork();
        if (pid == 0){
            printf("Created client #%d pid: %d %s\n",i,getpid(),get_time());
            signal(SIGINT,handler);
            key_t client_key = ftok(PATH,getpid());
            if ((client_semaphore = semget(client_key,1,IPC_CREAT | IPC_EXCL | 0666)) < 0){
                perror(strerror(errno));
                exit(EXIT_SUCCESS);
            }
            semctl(client_semaphore,0,SETVAL, 0);
            struct request buffer;
            buffer.pid = getpid();
            buffer.semaphore = client_semaphore;
            for (int j = 0; j < handle_number; j++){
                int queue_size;
                if ((queue_size = set_elem(clients,&buffer,server_semaphore)) == 0){
                    printf("Server wakes %s\n",get_time());
                }
                printf("Client %d wait #%d left %d %s\n",getpid(),j+1,queue_size,get_time());
                struct sembuf op_buffer;
                op_buffer.sem_flg = 0;
                op_buffer.sem_op = -1;
                op_buffer.sem_num = 0;
                semop(client_semaphore,&op_buffer,1);
                printf("Client %d begin #%d %s\n",getpid(),j+1,get_time());
                semop(client_semaphore,&op_buffer,1);
                printf("Client %d end #%d %s\n",getpid(),j+1,get_time());
                sleep(2);
            }
            printf("End client #%d pid: %d %s\n",i+1,getpid(),get_time());
            semctl(client_semaphore,0,IPC_RMID);
            exit(EXIT_SUCCESS);
        }
    }
    for (int i = 0; i < client_number; i++){
        wait(NULL);
    }
    return 0;
}


int set_elem(struct clients_queue *queue, struct request *buffer, int semaphore){
    struct sembuf op_buffer;
    op_buffer.sem_flg = 0;
    op_buffer.sem_op = -1;
    op_buffer.sem_num = 1;
    semop(semaphore,&op_buffer,1);
    int size = queue->size;
    if (queue->size >= MAX_CLIENT_NUMBER-1) return -1;
    queue->end++;
    if (queue->end == MAX_CLIENT_NUMBER) queue->end = 0;
    memcpy(&queue->clients[queue->end],buffer,sizeof(struct request));
    queue->size++;
    op_buffer.sem_op = 1;
    op_buffer.sem_num = 1;
    semop(semaphore,&op_buffer,1);
    op_buffer.sem_op = 1;
    op_buffer.sem_num = 0;
    semop(semaphore,&op_buffer,1);
    return size;
}