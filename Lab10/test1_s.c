#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

int main(){
    int sock_fd = -1;
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1){
        perror("socket");
        return 1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    char *path = "/tmp/server_sock";
    unlink(path);
    strcpy(addr.sun_path, path);
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1){
        perror("bind");
        return 1;
    }

    char *buffer = malloc(2048);
    printf("Wait for client\n");
    while(1){
        recvfrom(sock_fd, buffer, 10, 0, NULL, NULL);
        printf("%s\n", buffer);
        if (strcmp(buffer,"end") == 0) break;
    }
    close(sock_fd);
    free(buffer);
    return 0;
}