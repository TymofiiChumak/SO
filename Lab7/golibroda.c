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
#include "util.c"

key_t server_memory_key;
int server_memory;
key_t server_semaphore_key;
int server_semaphore;


int init(){
    server_memory_key = ftok(PATH,1);
    int server_memory_size = sizeof(struct clients_queue) * MAX_CLIENT_NUMBER;
    if ((server_memory = shmget(server_memory_key,server_memory_size,IPC_CREAT | IPC_EXCL | 0666)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    key_t server_semaphore_key = ftok(PATH,2);
    if ((server_semaphore = semget(server_semaphore_key,2,IPC_CREAT | IPC_EXCL | 0666)) < 0){
        perror(strerror(errno));
        shmctl(server_memory,IPC_RMID,NULL);
        exit(EXIT_SUCCESS);
    }
    perror(strerror(errno));    
    semctl(server_semaphore,0,SETVAL,0);
    semctl(server_semaphore,1,SETVAL,1);
    struct clients_queue queue;
    queue.size = 0;
    queue.begin = 0;
    queue.end = -1;
    struct clients_queue *dest = shmat(server_memory,NULL,0);
    memcpy(dest,&queue,sizeof(struct clients_queue));
}

int get_elem(struct clients_queue *queue, struct request *buffer, int semaphore);

void handle_one(){
    struct request *buffer;
    buffer = malloc(sizeof(struct request));    
    struct clients_queue *queue_buffer = (struct clients_queue*)shmat(server_memory,NULL,0);
    get_elem(queue_buffer,buffer,server_semaphore);
    printf("Start serve client with pid: %d %s\n",buffer->pid,get_time());
    struct sembuf op_buffer;
    op_buffer.sem_flg = 0;
    op_buffer.sem_op = 1;
    op_buffer.sem_num = 0;
    semop(buffer->semaphore,&op_buffer,1);
    sleep(1);
    printf("End serve client with pid: %d %s\n",buffer->pid,get_time());
    semop(buffer->semaphore,&op_buffer,1);
}

void handler(int signo){
    printf("Received interruption signal\n");
    semctl(server_semaphore,IPC_RMID,0);
    shmctl(server_memory,IPC_RMID,NULL);
    exit(EXIT_SUCCESS);

}

int main(int argc, char **argv){
    signal(SIGINT,handler);
    signal(SIGTERM,handler);
    init();
    while(1){
        handle_one();
    }
    
    raise(SIGINT);
}


int get_elem(struct clients_queue *queue, struct request *buffer, int semaphore){
    struct sembuf op_buffer;
    op_buffer.sem_flg = 0;
    op_buffer.sem_op = -1;
    op_buffer.sem_num = 0;
    if (queue->size == 0){
        printf("Server sleeps %s\n",get_time());
        semop(semaphore,&op_buffer,1);
        printf("Server wakes %s\n",get_time());
    }else{
        semop(semaphore,&op_buffer,1);
    }
    memcpy(buffer,&queue->clients[queue->begin],sizeof(struct request));
    queue->begin++;
    if (queue->begin == MAX_CLIENT_NUMBER) queue->begin = 0;
    queue->size--;    
    return 0;
}