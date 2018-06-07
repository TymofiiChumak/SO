#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "util.c"

#define  _GNU_SOURCE
#define MAX_CLIENTS 100
#define WAIT 1
#define NO_WAIT 0

int end;
key_t server_key;
int server_queue;
int clients_size = 0;
int *client_queues;
pid_t *pids;
int is_empty;
int fd;

void handler(int signo){
    printf("\nEnd of Server program\n");
    for (int i = 0; i < clients_size; i++){
        msgctl(client_queues[i],IPC_RMID,NULL);    
    }
    msgctl(server_queue,IPC_RMID,NULL);
    remove("/tmp/keyfifo");
    exit(EXIT_SUCCESS);
}

void send_begin(struct buf_msg_mirror *buffer){
    struct buf_msg_begin *msg_rcv = (struct buf_msg_begin*)buffer;
    printf("Recieved from %d MSG BEGIN\n",msg_rcv->pid);
    if (clients_size >= MAX_CLIENTS){ 
        perror("Too much clients\n");
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    int client_queue = msgget(msg_rcv->key, 0);
    if (client_queue < 0){
        perror(strerror(errno));
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    client_queues[clients_size] = client_queue;
    pids[clients_size] = msg_rcv->pid;
    clients_size ++;
    struct buf_msg_ident msg_ident;
    msg_ident.msgtype = MSG_IDENT;
    msg_ident.index = clients_size - 1;
    if (msgsnd(client_queue,&msg_ident,sizeof(int)*2,0) < 0){
        perror(strerror(errno));
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }

}

void send_mirror(struct buf_msg_mirror *msg_rcv){
    struct buf_msg_reply msg_reply;
    if (client_queues[msg_rcv->index] == -1) return;
    printf("Recieved from %d MSG MIRROR\n",pids[msg_rcv->index]);
    msg_reply.msgtype = MSG_MIRROR;
    sprintf(msg_reply.text, "%s",reverse(msg_rcv->text));
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    printf("Sent to %d mirror %s\n",pids[msg_rcv->index],msg_reply.text);
    if (msgsnd(client_queues[msg_rcv->index],&msg_reply,sizeof(int) + MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
}

int solve(int a, int b, int op){
    switch (op){
        case CALC_ADD:
            return a + b;
        case CALC_SUB:
            return a - b;
        case CALC_MUL:
            return a * b;
        case CALC_DIV:
            return a / b;
    }
}

void send_calc(struct buf_msg_mirror *buffer){
    struct buf_msg_calc *msg_rcv = (struct buf_msg_calc*)buffer;
    if (client_queues[msg_rcv->index] == -1) return;
    printf("Recieved from %d MSG CALC\n",pids[msg_rcv->index]);
    struct buf_msg_reply msg_reply;
    msg_reply.msgtype = MSG_CALC;
    sprintf(msg_reply.text, "%d\0",solve(msg_rcv->a,msg_rcv->b,msg_rcv->type));
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    printf("Sent to %d calc %s\n",pids[msg_rcv->index],msg_reply.text);
    if (msgsnd(client_queues[msg_rcv->index],&msg_reply,40,0) < 0){
        perror(strerror(errno));
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
}

void send_time(struct buf_msg_mirror *buffer){
    struct buf_msg_time *msg_rcv = (struct buf_msg_time*)buffer;
    if (client_queues[msg_rcv->index] == -1) return;
    printf("Recieved from %d MSG TIME\n",pids[msg_rcv->index]);
    struct buf_msg_reply msg_reply;
    msg_reply.msgtype = MSG_CALC;
    char *date_buffer = malloc(1024*sizeof(char));
    FILE *date_pipe = popen("date","r");
    fread(date_buffer,sizeof(char),1024,date_pipe);
    sprintf(msg_reply.text, "%s",date_buffer);
    pclose(date_pipe);
    free(date_buffer);
    printf("Sent to %d time %s\n",pids[msg_rcv->index],msg_reply.text);
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    if (msgsnd(client_queues[msg_rcv->index],&msg_reply,40,0) < 0){
        perror(strerror(errno));
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
}

void send_end(struct buf_msg_mirror *buffer){
    struct buf_msg_end *msg_rcv = (struct buf_msg_end*)buffer;
    if (client_queues[msg_rcv->index] == -1) return;
    printf("Recieved from %d MSG END\n",pids[msg_rcv->index]);
    struct buf_msg_reply msg_reply;
    msg_reply.msgtype = MSG_END;
    sprintf(msg_reply.text, "End of server program\n");
    end = 1;
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    if (msgsnd(client_queues[msg_rcv->index],&msg_reply,sizeof(int),0) < 0){
        perror(strerror(errno));
        msgctl(server_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
}

void delete_client(struct buf_msg_mirror *buffer){
    struct buf_msg_end *msg_rcv = (struct buf_msg_end*)buffer;
    printf("Recieved from %d MSG STOP\n",pids[msg_rcv->index]);
    client_queues[msg_rcv->index] = -1;
}

void read_msg(int wait){
    struct buf_msg_mirror buffer;
    if (wait == NO_WAIT){
        if (msgrcv(server_queue,&buffer,MAX_MIRROR_SIZE,0,IPC_NOWAIT | MSG_NOERROR) < 0){
            if (errno == ENOMSG){
                is_empty = 1;
                printf("EMPTY\n");
                return;
            }
            perror(strerror(errno));
            msgctl(server_queue,IPC_RMID,NULL);
            exit(EXIT_FAILURE);
        }        
    }else{
        if (msgrcv(server_queue,&buffer,MAX_MIRROR_SIZE,0,MSG_NOERROR) < 0){
            printf("OK2\n");            
            perror(strerror(errno));
            msgctl(server_queue,IPC_RMID,NULL);
            exit(EXIT_FAILURE);
        }
    }
    switch (buffer.msgtype){
        case MSG_BEGIN:
            send_begin(&buffer);
            break;
        case MSG_MIRROR:
            send_mirror(&buffer);
            break;
        case MSG_CALC:
            send_calc(&buffer);
            break;
        case MSG_TIME:
            send_time(&buffer);
            break;
        case MSG_END:
            send_end(&buffer);
            break;
        case MSG_STOP:
            delete_client(&buffer);
            break;
    }
}



int main(){
    end = 0;
    server_key =  ftok(KEY_PATH,PROJ_ID);
    /*mkfifo("/tmp/keyfifo",0666 );
    fd = open("/tmp/keyfifo",O_WRONLY);
    perror(strerror(errno));
    printf("%ld\n",server_key);
    write(fd,&server_key,sizeof(key_t));
    write(fd,0,sizeof(int));
    perror(strerror(errno));*/
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);
    msgctl(server_key,IPC_RMID,NULL);
    server_queue = msgget(server_key, 0666 |IPC_CREAT|IPC_EXCL);
    client_queues = malloc(sizeof(int) * MAX_CLIENTS);
    pids = malloc(sizeof(key_t) * MAX_CLIENTS);
    while(end == 0){
        read_msg(WAIT);
    }
    is_empty = 0;
    while(is_empty == 0){
        read_msg(NO_WAIT);
    }
    raise(SIGINT);
    return 0;
}