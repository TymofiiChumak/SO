#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include "util.h"

#define _GNU_SOURCE 
#define BUFFER_LENGTH    250
#define FALSE              0
#define MAX_EVENT         20

int server_socket_local;
int server_socket_inet;
int epoll_fd;
int recv_msg_num = 0;

struct client{
    char name[CLIENT_NAME_LENGTH];
    int socket_fd;
    int local;
};

struct client clients[CLIENTS_NUM];
int clients_size = 0;

int insert_client(char *name, int socket_fd, int local){
    int insert_index = -1;
    for (int i = 0; i < CLIENTS_NUM; i++){
        if (strcmp(name, clients[i].name) == 0 && clients[i].socket_fd != -1) return -1;
        if (clients[i].socket_fd == -1 && insert_index == -1) insert_index = i; 
    }
    strcpy(clients[insert_index].name, name);
    clients[insert_index].socket_fd = socket_fd;
    clients[insert_index].local = local;
    clients_size++;
    return 0;
}

void remove_client(int socket_fd){
    for (int i = 0; i < CLIENTS_NUM; i++){
        if (clients[i].socket_fd == socket_fd) {
            printf("remove %s\n", clients[i].name);
            clients[i].socket_fd = -1; 
            clients_size--;
        }
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, NULL);
}

int get_rand_client(){
    int index = -1;
    int is_empty = 1;
    for (int i = 0; i < CLIENTS_NUM; i++){
        if (clients[index].socket_fd != -1) {
            is_empty = 0;
            break;
        }
    }
    if (is_empty) return -1;
    do{
        index = rand() % CLIENTS_NUM;
    }while(clients[index].socket_fd == -1);
    return index;
}

sem_t received_ping;

struct command_queue{
    struct msg_arth wait_commands[BUFFER_LENGTH];
    sem_t queue_lock;
    sem_t full;
    sem_t empty;
    int last_get;
    int last_put;
}queue;

void get_elem(struct msg_arth *command){
    sem_wait(&queue.empty);
    sem_wait(&queue.queue_lock);
    queue.last_get++;
    if (queue.last_get >= BUFFER_LENGTH) queue.last_get++;
    *command = queue.wait_commands[queue.last_get];
    sem_post(&queue.full);
    sem_post(&queue.queue_lock);
}
    
void set_elem(struct msg_arth *command){
    sem_wait(&queue.full);
    sem_wait(&queue.queue_lock);
    queue.last_put++;
    if (queue.last_put >= BUFFER_LENGTH) queue.last_put++;
    queue.wait_commands[queue.last_put] = *command;
    sem_post(&queue.empty);
    sem_post(&queue.queue_lock);
}   

void *receiver(void *args){
    struct epoll_event *events = malloc(sizeof(struct epoll_event) * MAX_EVENT);
    int events_num;
    while(1){
        memset(events, 0, MAX_EVENT * sizeof(struct epoll_event));
        events_num = epoll_wait(epoll_fd, events, MAX_EVENT, -1);
        for (int i = 0; i < events_num; i++){
            int tmp_sock = events[i].data.fd;
            if (tmp_sock == server_socket_inet || tmp_sock == server_socket_local){
                struct sockaddr client_addr;
                socklen_t addr_len;
                int client_sock = accept(tmp_sock, (struct sockaddr *)&client_addr, &addr_len);
                printf("client %d\n", client_sock);
                struct epoll_event ee;
                ee.data.fd = client_sock;
                ee.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &ee);
            }else{
                int type = -1;
                recv(tmp_sock, &type, sizeof(int), MSG_WAITALL);
                printf("type %d\n",type);
                
                if (type == MSG_TYPE_ADD){
                
                    struct msg_client_name msg_buffer;
                    int ret_val = recv(tmp_sock, &msg_buffer.client_name, sizeof(struct msg_client_name) - sizeof(int), MSG_WAITALL);
                    printf("add [%s]\n", msg_buffer.client_name);
                    struct msg_response response;
                    response.type = MSG_TYPE_ADD_RESPONSE;
                    int local = tmp_sock == server_socket_local;
                    response.result = insert_client(msg_buffer.client_name, tmp_sock, local);
                    if (response.result == -1){
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, tmp_sock, NULL);
                        printf("Can't add %s\n", msg_buffer.client_name);
                    }
                    send(tmp_sock, &response, sizeof(struct msg_response), 0);
                
                }else if (type == MSG_TYPE_ARTH_RESPONSE){
                
                    struct msg_response msg_buffer;
                    recv(tmp_sock, &msg_buffer.result, sizeof(struct msg_response) - sizeof(int), MSG_WAITALL);
                    printf("Result #%d: %d\n",recv_msg_num, msg_buffer.result);                
                    recv_msg_num++;
                
                }else if (type == MSG_TYPE_PING_RESPONSE){
                    struct msg_response msg_buffer;
                    recv(tmp_sock, &msg_buffer.result, sizeof(struct msg_response) - sizeof(int), MSG_WAITALL);
                    printf("received ping response\n");
                    sem_post(&received_ping);
                }else if (type == MSG_TYPE_REMOVE){
                    remove_client(tmp_sock);
                }else{
                    printf("unknown type\n");
                    raise(SIGINT);
                }
            }            
        }
    }
}

void *command_receiver(void *args){
    int a, b;
    char op[2];
    while (1){
        scanf("%d",&a);
        scanf("%s",op);
        scanf("%d",&b);
        
        struct msg_arth buffer;
        buffer.type = MSG_TYPE_ARTH;
        buffer.a = a;
        buffer.b = b;
        switch(op[0]){
            case '+': 
                buffer.op = OP_ADD;
                break;
            case '-': 
                buffer.op = OP_SUB;
                break;
            case '*': 
                buffer.op = OP_MUL;
                break;
            case '/': 
                buffer.op = OP_DIV;
                break;
            default: 
                perror("wrong operation");
                continue;
        }
        int index = get_rand_client();
        if (index == -1){
            perror("There is no clients");
            continue;
        }
        send(clients[index].socket_fd, &buffer, sizeof(buffer), 0);
        printf("sended arth: %d %d %d\n",buffer.a, buffer.op, buffer.b);
    }
    return 0;
}


void *ping_clients(void *args){
    struct msg_response check_msg;
    check_msg.type = MSG_TYPE_PING;
    while(1){
        sleep(5);
        for (int i = 0; i < CLIENTS_NUM; i++){
            if(clients[i].socket_fd == -1) continue;
            send(clients[i].socket_fd, &check_msg, sizeof(struct msg_response), 0);
            printf("ping client: %s\n", clients[i].name);
            struct timespec t;
            clock_gettime(CLOCK_REALTIME, &t);
            t.tv_sec += 1;
            sem_timedwait(&received_ping, &t);
            if (errno == ETIMEDOUT){
                remove_client(clients[i].socket_fd);
            }
        }
    }
}   

void init(int port, char *path){
    server_socket_local = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_local < 0){
        perror("local socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un addr_local;
    addr_local.sun_family = AF_UNIX;
    strcpy(addr_local.sun_path, path);
    unlink(path);
    if (bind(server_socket_local, (struct sockaddr *)&addr_local, sizeof(struct sockaddr_un)) < 0){
        perror("local bind");
        exit(EXIT_FAILURE);
    }
    listen(server_socket_local, CLIENTS_NUM);

    server_socket_inet = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_inet < 0){
        perror("inet socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in addr_inet;
    memset(&addr_inet, 0, sizeof(addr_inet));
    addr_inet.sin_family = AF_INET;
    addr_inet.sin_port = htons(port);
    addr_inet.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Address: %s\n", inet_ntoa(addr_inet.sin_addr));
    if (bind(server_socket_inet, (struct sockaddr *)&addr_inet, sizeof(struct sockaddr_in)) == -1){
        perror("inet bind");
        exit(EXIT_FAILURE);
    }
    listen(server_socket_inet, CLIENTS_NUM);
    
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0){
        perror("epoll fd");
        exit(EXIT_FAILURE);
    } 
    struct epoll_event event_local;
    event_local.events = EPOLLIN; 
    event_local.data.fd = server_socket_local;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_local, &event_local);

    struct epoll_event event_inet;
    event_inet.events = EPOLLIN;
    event_inet.data.fd = server_socket_inet;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_inet, &event_inet);

    for (int i = 0; i < CLIENTS_NUM; i++){
        clients[i].socket_fd = -1;
    }
}

void handler(int signo){
    printf("received interrupt signal\n");
    close(server_socket_inet);
    close(server_socket_local);
    close(epoll_fd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    if (argc < 2){
        perror("Too few args");
        exit(EXIT_FAILURE);
    }
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sigaction(SIGINT, &sa, NULL);
    init(atoi(argv[1]), argv[2]);
    printf("End init\n Ready to connect with clients\n");
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_t command_thread;
    pthread_create(&command_thread, NULL, command_receiver, NULL);
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_clients, NULL);
    pthread_join(receiver_thread, NULL);    
    pthread_join(ping_thread, NULL);    
    pthread_join(command_thread, NULL);
}