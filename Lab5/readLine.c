#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
    
    while(1){
        char buffer[1024];
        scanf("%s",buffer);
        if (argc != 1){
            printf("%s%s\n",buffer,argv[1]);
        }else{
            printf("%s\n",buffer);
        }
        if (strcmp(buffer,"exit") == 0) return 0 ;
    }
    return 0;
}