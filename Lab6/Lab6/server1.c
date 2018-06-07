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
#include <mqueue.h>
#include <fcntl.h>
#include "util1.c"


#define  _GNU_SOURCE
#define MAX_CLIENTS 100
#define WAIT 1
#define NO_WAIT 0

int end;
mqd_t server_queue;
int clients_size = 0;
mqd_t *client_queues;
pid_t *pids;
int is_empty;
int fd;

void handler(int signo){
    printf("\nEnd of Server program\n");
    for (int i = 0; i < clients_size; i++){
        mq_close(client_queues[i]);
    }
    mq_close(server_queue);
    mq_unlink("/serverqueue");
    exit(EXIT_SUCCESS);
}

void send_begin(struct buf_msg_mirror *buffer){
    struct buf_msg_begin *msg_rcv = (struct buf_msg_begin*)buffer;
    printf("Recieved from %d MSG BEGIN\n",msg_rcv->pid);
    if (clients_size >= MAX_CLIENTS){ 
        perror("Too much clients\n");
        mq_close(server_queue);
        exit(EXIT_FAILURE);
    }
    int client_queue = mq_open(msg_rcv->queue_name, O_WRONLY);
    if (client_queue < 0){
        perror(strerror(errno));
        mq_close(server_queue);
        exit(EXIT_FAILURE);
    }
    client_queues[clients_size] = client_queue;
    pids[clients_size] = msg_rcv->pid;
    clients_size ++;
    struct buf_msg_ident msg_ident;
    msg_ident.msgtype = MSG_IDENT;
    msg_ident.index = clients_size - 1;
    if (mq_send(client_queue,(char*)&msg_ident,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(server_queue);
        exit(EXIT_FAILURE);
    }

}

void send_mirror(struct buf_msg_mirror *msg_rcv){
    struct buf_msg_reply msg_reply;
    if (client_queues[msg_rcv->index] == -1) return;
    printf("Recieved from %d MSG MIRROR\n",pids[msg_rcv->index]);
    msg_reply.msgtype = MSG_MIRROR;
    sprintf(msg_reply.text, "%s\0",reverse(msg_rcv->text));
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    printf("Sent to %d mirror %s\n",pids[msg_rcv->index],msg_reply.text);
    if (mq_send(client_queues[msg_rcv->index],(char*)&msg_reply,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(server_queue);
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
    printf("Sent to %d calc %s\0\n",pids[msg_rcv->index],msg_reply.text);
    if (mq_send(client_queues[msg_rcv->index],(char*)&msg_reply,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(server_queue);
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
    sprintf(msg_reply.text, "%s\0",date_buffer);
    pclose(date_pipe);
    free(date_buffer);
    printf("Sent to %d time %s\n",pids[msg_rcv->index],msg_reply.text);
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    if (mq_send(client_queues[msg_rcv->index],(char*)&msg_reply,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(server_queue);
        exit(EXIT_FAILURE);
    }
}

void send_end(struct buf_msg_mirror *buffer){
    struct buf_msg_end *msg_rcv = (struct buf_msg_end*)buffer;
    end = 1;
    printf("Recieved from %d MSG END\n",pids[msg_rcv->index]);
    if (client_queues[msg_rcv->index] == -1) return;
    struct buf_msg_reply msg_reply;
    msg_reply.msgtype = MSG_END;
    sprintf(msg_reply.text, "End of server program\0");
    msg_reply.rcv_msgtype = msg_rcv->msgtype;
    if (mq_send(client_queues[msg_rcv->index],(char*)&msg_reply,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(server_queue);
        exit(EXIT_FAILURE);
    }
}

void delete_client(struct buf_msg_mirror *buffer){
    struct buf_msg_end *msg_rcv = (struct buf_msg_end*)buffer;
    printf("Recieved from %d MSG STOP\n",pids[msg_rcv->index]);
    client_queues[msg_rcv->index] = -1;
}

void read_msg(){
    struct buf_msg_mirror buffer;
    if (mq_receive(server_queue,(char*)&buffer,8192,0) < 0){
        if (errno == ENOMSG){
            is_empty = 1;
            return;
        }
        perror(strerror(errno));
        mq_close(server_queue);
        exit(EXIT_FAILURE);
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
    }
}



int main(){
    end = 0;
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);
    mode_t mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    struct mq_attr server_attr;
    server_attr.mq_flags = 0;
    server_attr.mq_curmsgs = 0;
    server_attr.mq_maxmsg = MAX_MASSAGE_NUMBER;
    server_attr.mq_msgsize = MAX_MIRROR_SIZE;
    mq_unlink("/serverqueue");
    server_queue = mq_open("/serverqueue", O_CREAT | O_RDWR | O_EXCL, 0666, NULL);
    if (server_queue < 0){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct mq_attr a;
    mq_getattr(server_queue,&a);
    printf("%d\n",a.mq_msgsize);
    client_queues = malloc(sizeof(mqd_t) * MAX_CLIENTS);
    pids = malloc(sizeof(pid_t) * MAX_CLIENTS);
    printf("Ready\n");
    while(end == 0){
        read_msg(WAIT);
    }
    is_empty = 0;
    if (mq_getattr(server_queue,&server_attr) < 0){
        printf("getattr\n");
        perror(strerror(errno));
    }
    server_attr.mq_flags = O_NONBLOCK;
    if (mq_setattr(server_queue,&server_attr,NULL) < 0){
        printf("setattr\n");
        perror(strerror(errno));
    }
    raise(SIGINT);
    printf("----\n");
    while(is_empty == 0){
        read_msg(NO_WAIT);
    }
    raise(SIGINT);
    return 0;
}