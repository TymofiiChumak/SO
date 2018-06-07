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

int client_queue = -1;
key_t client_key;
int server_queue;
key_t server_key;
int client_index;


void send_stop(){
    struct buf_msg_end msg_end;
    msg_end.msgtype = MSG_STOP;
    msg_end.index = client_index;
    if (msgsnd(server_queue,&msg_end,MAX_MIRROR_SIZE,0) < 0){
        if (errno == EINVAL) perror("Server queue was closed");
        perror(strerror(errno));
        msgctl(client_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    printf("Sent STOP\n");
}

void handler(int signo){
    if (signo == SIGINT){
        printf("Received SIGINT\n");
        if (client_queue != -1){
            send_stop();
            msgctl(client_queue,IPC_RMID,NULL);
        }
        exit(EXIT_SUCCESS);
    }
}


void send_mirror(char *text){
    struct buf_msg_mirror msg_mirror;
    msg_mirror.msgtype = MSG_MIRROR;
    msg_mirror.index = client_index;
    sprintf(msg_mirror.text, "%s", text);
    if (msgsnd(server_queue,&msg_mirror,MAX_MIRROR_SIZE,0) < 0){
        if (errno == EINVAL) perror("Server queue was closed");
        perror(strerror(errno));
        msgctl(client_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    printf("Sent mirror: %s\n",text);
}

void send_calc(char *text){
    struct buf_msg_calc msg_calc;
    msg_calc.msgtype = MSG_CALC;
    msg_calc.index = client_index;
    int *res = parse_calc(text);
    msg_calc.a = res[0];
    msg_calc.b = res[1];
    msg_calc.type = res[2];
    if (msgsnd(server_queue,&msg_calc,MAX_MIRROR_SIZE,0) < 0){
        if (errno == EINVAL) perror("Server queue was closed");
        perror(strerror(errno));
        msgctl(client_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    printf("Sent calc: %s\n",text);
}

void send_time(){
    struct buf_msg_time msg_time;
    msg_time.msgtype = MSG_TIME;
    msg_time.index = client_index;
    if (msgsnd(server_queue,&msg_time,MAX_MIRROR_SIZE,0) < 0){
        if (errno == EINVAL) perror("Server queue was closed");
        perror(strerror(errno));
        msgctl(client_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    printf("Sent time\n");
}

void send_end(){
    struct buf_msg_end msg_end;
    msg_end.msgtype = MSG_END;
    msg_end.index = client_index;
    if (msgsnd(server_queue,&msg_end,MAX_MIRROR_SIZE,0) < 0){
        if (errno == EINVAL) perror("Server queue was closed");
        perror(strerror(errno));
        msgctl(client_queue,IPC_RMID,NULL);
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
        command_number ++;
        if (strcmp(components[0],"MIRROR") == 0){
            send_mirror(components[1]);
        }else if (strcmp(components[0],"CALC") == 0){
            send_calc(components[1]);
        }else if (strcmp(components[0],"TIME") == 0){
            send_time();
        }else if (strcmp(components[0],"END") == 0){
            send_end();
            break;
        }
        
    }
    return command_number;
}

void receiver(int command_number){
    for (int i = 0; i < command_number ; i++){
        struct buf_msg_reply buffer;
        if (msgrcv(client_queue,&buffer,MAX_MIRROR_SIZE,0,MSG_NOERROR) < 0){
            if (errno == EINVAL) perror("Server queue was closed");
            perror(strerror(errno));
            msgctl(client_queue,IPC_RMID,NULL);
            exit(EXIT_FAILURE);
        }
        char *type;
        switch (buffer.rcv_msgtype){
            case MSG_MIRROR: type = "MIRROR"; break;
            case MSG_CALC: type = "CALC"; break;
            case MSG_TIME: type = "TIME"; break;
            case MSG_END: type = "END"; break;
        }
        printf("Received %s\n   %s\n",type,buffer.text);
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
    FILE *source = fopen(argv[1],"r");
    if (source < 0){
        printf("--1\n");                
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    /*
    int key_pipe = open("/tmp/keyfifo",O_RDWR);
    read(key_pipe,&server_key,sizeof(key_t));
    int num;
    read(key_pipe,&num,sizeof(int));
    num++;
    write(key_pipe,&server_key,sizeof(key_t));
    write(key_pipe,&num,sizeof(int));*/
    int num = 1;
    server_key =  ftok(KEY_PATH,PROJ_ID);
    client_key = ftok(KEY_PATH,getpid());
    client_queue = msgget(client_key,0666 |IPC_CREAT|IPC_EXCL);
    if (client_queue < 0){
        printf("--2\n");               
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    server_queue = msgget(server_key,0);
    if (server_queue < 0){
        printf("--3\n");                
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct buf_msg_begin msg_begin;
    msg_begin.key = client_key;
    msg_begin.msgtype = MSG_BEGIN;
    msg_begin.pid = getpid();

    if (msgsnd(server_queue,&msg_begin,MAX_MIRROR_SIZE,0) < 0){
        perror(strerror(errno));
        printf("--4\n");                
        msgctl(client_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }

    struct buf_msg_ident msg_ident;
    if (msgrcv(client_queue,&msg_ident,MAX_MIRROR_SIZE,0,0) < 0){
        perror(strerror(errno));
        printf("--5\n");    
        msgctl(client_queue,IPC_RMID,NULL);
        exit(EXIT_FAILURE);
    }
    client_index = msg_ident.index;
    int command_number = sender(source);
    receiver(command_number);
    msgctl(client_queue,IPC_RMID,NULL);
    fclose(source);
    return 0;
}