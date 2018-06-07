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
#include "util1.c"

mqd_t client_queue;
mqd_t server_queue;
int client_index;
char *path;
FILE *source;

void send_stop(){
    struct buf_msg_end msg_end;
    msg_end.msgtype = MSG_STOP;
    msg_end.index = client_index;
    if (mq_send(server_queue,(char*)&msg_end,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(client_queue);
        exit(EXIT_FAILURE);
    }
    printf("Sent STOP\n");
}

void handler(int signo){
    if (signo == SIGINT){
        printf("Received SIGINT\n");
        send_stop();
        mq_close(client_queue);
        exit(EXIT_SUCCESS);
    }
}

void send_mirror(char *text){
    struct buf_msg_mirror *msg_mirror;
    msg_mirror = malloc(sizeof(struct buf_msg_mirror));
    msg_mirror->msgtype = MSG_MIRROR;
    msg_mirror->index = client_index;
    sprintf(msg_mirror->text, "%s", text);
    if (mq_send(server_queue,(char*)msg_mirror,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(client_queue);
        exit(EXIT_FAILURE);
    }
    printf("Sent mirror: %s\n",text);
}

void send_calc(char *text){
    struct buf_msg_calc *msg_calc;
    msg_calc = malloc(sizeof(struct buf_msg_calc));
    msg_calc->msgtype = MSG_CALC;
    msg_calc->index = client_index;
    int *res = parse_calc(text);
    msg_calc->a = res[0];
    msg_calc->b = res[1];
    msg_calc->type = res[2];
    if (mq_send(server_queue,(char*)msg_calc,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(client_queue);
        exit(EXIT_FAILURE);
    }
    printf("Sent calc: %s\n",text);
}

void send_time(){
    struct buf_msg_time *msg_time;
    msg_time = malloc(sizeof(struct buf_msg_time));
    msg_time->msgtype = MSG_TIME;
    msg_time->index = client_index;
    if (mq_send(server_queue,(char*)msg_time,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(client_queue);
        exit(EXIT_FAILURE);
    }
    printf("Sent time\n");
}

void send_end(){
    struct buf_msg_end *msg_end;
    msg_end = malloc(sizeof(struct buf_msg_end));
    msg_end->msgtype = MSG_END;
    msg_end->index = client_index;
    if (mq_send(server_queue,(char*)msg_end,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        mq_close(client_queue);
        exit(EXIT_FAILURE);
    }
    printf("Sent end\n");
}

char **parse_line(char *line){
    int begin = 0;
    char **result = calloc(2,sizeof(char*));
    while (line[begin] == ' ' && line[begin] != '\0') begin++;
    result[0] = &line[begin];
    while (line[begin] != ' ' && line[begin] != '\0') begin++;
    line[begin] = '\0';
    begin++;
    while (line[begin] == ' ' && line[begin] != '\0') begin++;
    result[1] = &line[begin];
    return(result);
}

int sender(FILE *source){
    int command_number = 0;
    char c = ' ';
    while (c != EOF){
        char *line = calloc(MAX_MIRROR_SIZE,sizeof(char));
        int i = 0;
        while ((c = fgetc(source)) != EOF){
            if ((int)c == 10){
                line[i] = '\0';
                break;
            }else{
                line[i] = c;
                i++;
            }
            if (c == EOF){
                line[i] = '\0';
            }
        }
        if (strlen(line) < 1) continue;
        char **components = parse_line(line);
        sleep(1);
        if (strcmp(components[0],"MIRROR") == 0){
            send_mirror(components[1]);
        }else if (strcmp(components[0],"CALC") == 0){
            send_calc(components[1]);
        }else if (strcmp(components[0],"TIME") == 0){
            send_time(components[1]);
        }else if (strcmp(components[0],"END") == 0){
            send_end(components[1]);
        }
        free(line);
        free(components);
        command_number ++;
    }
    return command_number;
}

void receive_one(){
        struct buf_msg_reply *buffer;
        buffer = malloc(sizeof(struct buf_msg_reply));
        if (mq_receive(client_queue,(char*)buffer,MAX_MIRROR_SIZE,0) < 0){
            perror(strerror(errno));
            mq_close(client_queue);
            exit(EXIT_FAILURE);
        }
        char *type;
        switch (buffer->rcv_msgtype){
            case MSG_MIRROR: type = "MIRROR"; break;
            case MSG_CALC: type = "CALC"; break;
            case MSG_TIME: type = "TIME"; break;
            case MSG_END: type = "END"; break;
        }
        printf("Received %s   %s\n",type,buffer->text);
        free(buffer);
    
}

void receiver(int command_number){
    for (int i = 0; i < command_number ; i++){
        receive_one();
    }
}


int main(int argc, char **argv){
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);
    if (argc < 2){
        perror("Too few args\n");
        exit(EXIT_FAILURE);
    }
    source = fopen(argv[1],"r");
    if (source < 0){
        printf("--1\n");                
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    path = malloc(sizeof(char) * 64);
    sprintf(path,"/clientqueue%d",getpid());

    client_queue = mq_open(path, O_CREAT | O_RDONLY | O_EXCL, 0666, NULL);
    if (client_queue < 0){
        printf("--2\n");               
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    server_queue = mq_open("/serverqueue", O_RDWR);
    if (server_queue < 0){
        perror("Please run a server program\n");
        printf("--3\n");                
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct buf_msg_begin *msg_begin;
    msg_begin = malloc(sizeof(struct buf_msg_begin));
    msg_begin->msgtype = MSG_BEGIN;
    msg_begin->pid = getpid();
    sprintf(msg_begin->queue_name,"%s",path);

    if (mq_send(server_queue,(char*)msg_begin,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        printf("--4\n");                
        mq_unlink(path);
        exit(EXIT_FAILURE);
    }
    free(msg_begin);

    struct buf_msg_ident *msg_ident;
    msg_ident = malloc(sizeof(struct buf_msg_ident));
    if (mq_receive(client_queue,(char*)msg_ident,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        printf("--5\n");    
        mq_close(client_queue);
        exit(EXIT_FAILURE);
    }
    client_index = msg_ident->index;
    free(msg_ident);
    int command_number = sender(source);
    receiver(command_number);
    mq_close(client_queue);
    mq_close(server_queue);
    mq_unlink(path);
    return 0;
}