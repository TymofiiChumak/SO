#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *file = fopen("input.txt","r");
    char *buffer = malloc(10240);
    while (1){
        if (fscanf(file,"%[^\n]\n",buffer) == 0) break;
        printf("%s\n",buffer);
    }
    fclose(file);
    return 0;
}