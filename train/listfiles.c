#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char** argv){
    char *dir_name = argv[1];
    DIR *dir = opendir(dir_name);
    if( dir == NULL){
        perror("Error open\n");
    }
    struct dirent *info;
    while((info = readdir(dir)) != NULL){
        if (strcmp(info->d_name,".") != 0 && strcmp(info->d_name,"..") != 0 ){
            printf("%s\n",info->d_name);
        }
    }
    return 0;
}