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
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "util1.c"

int server_memory;
sem_t *server_semaphore1;
sem_t *server_semaphore2;
struct clients_queue *queue_buffer;

int init(){
    int server_memory_size = sizeof(struct clients_queue);
    if ((server_memory = shm_open("/servermem",O_CREAT | O_EXCL | O_RDWR,0666)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    ftruncate(server_memory,sizeof(struct clients_queue));
    if ((server_semaphore1 = sem_open("/serversem1",O_CREAT | O_EXCL,0666,0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    if ((server_semaphore2 = sem_open("/serversem2",O_CREAT | O_EXCL,0666,1)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    perror(strerror(errno));    
    queue_buffer = mmap(NULL,server_memory_size,PROT_READ | PROT_WRITE,MAP_SHARED,server_memory,0);
    queue_buffer->size = 0;
    queue_buffer->begin = 0;
    queue_buffer->end = -1;
}

int get_elem(struct request *buffer);

void handle_one(){
    struct request *buffer;
    buffer = malloc(sizeof(struct request));    
    get_elem(buffer);
    printf("Start serve client with pid: %d %s\n",buffer->pid,get_time());
    sem_t *client_semaphore;
    char *sem_name = malloc(32*sizeof(char));
    sprintf(sem_name,"/clientsem%d",buffer->index);
    if ((client_semaphore = sem_open(sem_name,0)) < 0){
        perror(strerror(errno));
        exit(EXIT_SUCCESS);
    }
    sem_post(client_semaphore);
    sleep(1);
    printf("End serve client with pid: %d %s\n",buffer->pid,get_time());
    sem_post(client_semaphore);
}

void handler(int signo){
    printf("Received interruption signal\n");
    sem_unlink("/serversem1");
    sem_unlink("/serversem2");
    shm_unlink("/servermem");
    exit(EXIT_SUCCESS);

}

int main(int argc, char **argv){
    signal(SIGINT,handler);
    signal(SIGTERM,handler);
    init();
    printf("$$ %d\n",queue_buffer->size);
    while(1){
        handle_one();
    }
    
    raise(SIGINT);
}


int get_elem(struct request *buffer){
    printf("3223\n");            
    if (queue_buffer->size == 0){
        printf("Server sleeps %s\n",get_time());
        sem_wait(server_semaphore1);
        printf("Server wakes %s\n",get_time());
    }else{
        printf("3\n");        
        sem_wait(server_semaphore1);
    }
    printf("3223\n");            
    memcpy(buffer,&queue_buffer->clients[queue_buffer->begin],sizeof(struct request));
    printf("3223\n");        
    
    queue_buffer->begin++;
    if (queue_buffer->begin == MAX_CLIENT_NUMBER) queue_buffer->begin = 0;
    queue_buffer->size--;
    
    return 0;
}