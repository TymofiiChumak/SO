#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(){
    int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1){
        perror("socket");
        return 1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/server_sock");
    bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));

    char *buffer = malloc(2048);
    sprintf(buffer, "Hello %d", getpid());
    if (sendto(sock_fd, buffer, 2048, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1){
        perror("bind");
        return 1;
    }
    free(buffer);
    close(sock_fd);
    return 0;
}