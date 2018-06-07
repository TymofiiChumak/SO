#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    int i = 4;
    double d = 4.0;
    char c = 'c';
    pid_t child_pid;
    child_pid = fork();
    if (child_pid == 0){
        printf("1: i = %d, d = %f , c = %c\n", i, d, c);
        i = 5;
        d = 5.0;
        c = 'f';
        printf("2: i = %d, d = %f , c = %c\n", i, d, c);
        exit(0);
    }else{
        while (wait(0) > 0);
        printf("3: i = %d, d = %f , c = %c\n", i, d, c);
    }

    return 0;
}