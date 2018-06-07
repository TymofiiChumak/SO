#include <stdio.h>
#include <unistd.h>
int main(int argc, char** argv){
    char path[1024];
    getcwd(path, sizeof(path));
    printf("Program runing with PATH=%s\n", path);
}