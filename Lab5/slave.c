#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

int file;
char* path;



int main(int argc, char **argv){
    path = argv[1];
    int n = strtol(argv[2],NULL,10);
    file = open(path,O_WRONLY);
    pid_t pid = getpid();
    srand(time(NULL));
    printf("Created slave proces PID: %d \n\n",pid);
    for (int i = 0; i < n ; i++){
        sleep(rand() % 4 + 1);
        char *date_buffer = malloc(1024*sizeof(char));
        FILE *date_pipe = popen("date","r");
        fread(date_buffer,sizeof(char),1024,date_pipe);
        char *buffer = malloc(1024*sizeof(char));
        sprintf(buffer,"PID: %d, Date: %s\n",pid,date_buffer);
        write(file,buffer,1024);
        pclose(date_pipe);
        free(date_buffer);
    }
    close(file);
    return(0);
}