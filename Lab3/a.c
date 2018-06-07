#include <stdio.h>

int main(int argc, char** argv){
    printf("Execute A with %d arguments\n",argc-1);
    for (int i = 1; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
}