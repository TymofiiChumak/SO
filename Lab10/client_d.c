#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include "util.h"

#define CON_LOCAL 0
#define CON_PORT 1

char *name;
int con_type;
int server_socket;

int solve(struct msg_arth *msg){
    switch(msg->op){
        case OP_ADD:
            return(msg->a + msg->b);
        case OP_SUB:
            return(msg->a - msg->b);
        case OP_MUL:
            return(msg->a * msg->b);
        case OP_DIV:
            return(msg->a / msg->b);
    }
}

void reply(){
    int type;
    recv(server_socket, &type, sizeof(int), MSG_WAITALL | MSG_PEEK);
    void *buffer;
    struct msg_response responce;
    switch(type){
        case MSG_TYPE_ARTH: 
            buffer = malloc(sizeof(struct msg_arth));
            recv(server_socket, buffer, sizeof(struct msg_arth), MSG_WAITALL); 
            responce.type = MSG_TYPE_ARTH_RESPONSE;
            responce.result = solve((struct msg_arth *)buffer);
            printf("received arth: %d\n", responce.result);
            break;
        case MSG_TYPE_PING:
            buffer = malloc(sizeof(struct msg_response));
            recv(server_socket, buffer, sizeof(struct msg_response), MSG_WAITALL); 
            responce.type = MSG_TYPE_PING_RESPONSE;
            responce.result = ((struct msg_response *)buffer)->result;
            printf("received ping\n");
            break;   
    }
    send(server_socket, &responce, sizeof(struct msg_response), 0);
    printf("sended\n");
}

void handler(int signo){
    if (signo == SIGINT) printf("Received inerrupt signal\n");
    if (signo == SIGPIPE) printf("Socket closed\n");
    struct msg_response msg_end;
    msg_end.type = MSG_TYPE_REMOVE;
    msg_end.result = 0;
    send(server_socket, &msg_end, sizeof(msg_end), 0);
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    if (argc < 3){
        perror("Too few args");
        return 1;
    }
    name = argv[1];
    
    if (strcmp(argv[2], "local") == 0){
        con_type = CON_LOCAL;
    }else if (strcmp(argv[2], "port") == 0){
        con_type = CON_PORT;
    }else{
        perror("Wrong connection type");
        return 1;
    }

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);

    if (con_type == CON_PORT){
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1){
            perror("server socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in sock_addr;
        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_port = htons(atoi(argv[4]));
        sock_addr.sin_addr.s_addr = inet_addr(argv[3]);
        if (connect(server_socket, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in)) == -1){
            perror("connect");
            exit(EXIT_FAILURE);
        }
    }else{
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket == -1){
            perror("server socket");
            exit(EXIT_FAILURE);
        }
        struct sockaddr_un sock_addr;
        sock_addr.sun_family = AF_UNIX;
        strcpy(sock_addr.sun_path, argv[3]);
        if (connect(server_socket, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_un)) == -1){
            perror("connect");
            exit(EXIT_FAILURE);
        }
    }

    struct msg_client_name msg_init;
    msg_init.type = MSG_TYPE_ADD;
    strcpy(msg_init.client_name, name);
    printf("%s\n", msg_init.client_name);
    send(server_socket, &msg_init, sizeof(struct msg_client_name), 0);
    struct msg_response response;
    recv(server_socket, &response, sizeof(struct msg_response), MSG_WAITALL); 
    if (response.result == -1){
        perror("Server can't add this client");
        shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
        return 1;
    }

    while(1){
        reply();
    }
}