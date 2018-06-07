#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

int file;
char* path;

void handler(int signo){
    if (signo == SIGINT){
        printf("\nExit from master program\n");
        close(file);
        unlink(path);
        exit(0);
    }
}


int main(int argc, char **argv){
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);
    path = argv[1];
    if (mkfifo(path,0644) != 0){
        if (errno == EEXIST){ 
            printf("Removing existing pipe\n");
            remove(path);
        }else{
            perror("Can't open a fifo\n");
            exit(0);
        }
    }
    file = open(path,O_RDONLY);
    printf("Conected first slave \n");            
    while(1){
        char buffer[1024];            
        int size = read(file,buffer,1024);
        if (size != 0) printf("%s",buffer);
    }
    raise(SIGINT);
}   