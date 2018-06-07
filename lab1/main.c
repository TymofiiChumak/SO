#include <math.h>
#include <sys/times.h>
#include <stdio.h>
#include "charBlocks.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> 

clock_t st_time;
clock_t en_time;
struct tms st_cpu;
struct tms en_cpu;
extern char** arrayB;

typedef int bool;
#define true 1
#define false 0

void
start_clock()
{
    st_time = times(&st_cpu);
}

void end_clock(char *msg, FILE *destFile)
{
    en_time = times(&en_cpu);
    fputs(msg,destFile);
    char buffer[1000] = {0};
    sprintf(buffer,"Real Time: %d, User Time: %f, System Time: %f\n",
        (en_time - st_time),
        (double)(en_cpu.tms_utime - st_cpu.tms_utime) / CLOCKS_PER_SEC,
        (double)(en_cpu.tms_stime - st_cpu.tms_stime) / CLOCKS_PER_SEC);
    fputs(buffer, destFile);
}




int main(int argc, char **argv){
    bool dynamic;
    int arraySize = 0;
    int blockSize = 0;
    FILE *destFile = fopen("raport2.txt","ab");
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i],"dynamic") == 0){
            dynamic = true;
        }else if (strcmp(argv[i],"static") == 0){
            dynamic = false;
        }else if (strcmp(argv[i],"asize")==0){
            arraySize = strtol(argv[i+1],NULL,10);
        }else if (strcmp(argv[i],"bsize")==0){
            blockSize = strtol(argv[i+1],NULL,10);        
        }else if (strcmp(argv[i],"create_table")==0){
            char buffer[1000] = {0};
            arraySize = strtol(argv[i+1],NULL,10);
            blockSize = strtol(argv[i+2],NULL,10);
            sprintf(buffer,"Creation %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                arrayB = (char**)calloc(arraySize,sizeof(char*));
                //creationArrayD(arraySize, blockSize);
            }else{
                char newArray[arraySize][blockSize];
                arrayB = (char**)newArray; 
                //creationArrayS(arraySize, blockSize);
            }
            end_clock(buffer,destFile);
        }else if (strcmp(argv[i],"delete")==0){
            char buffer[1000] = {0};
            sprintf(buffer,"Deletion %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                deleteArrayD(arraySize);
            }else{
                deleteArrayS(arraySize);
            }
            end_clock(buffer,destFile);
        }else if (strcmp(argv[i],"add") == 0){
            int number = strtol(argv[i+1],NULL,10);
            char buffer[1000] = {0};
            sprintf(buffer,"creation of block %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                addBlockD(arraySize, blockSize, number);
            }else{
                addBlockS(arraySize, blockSize, number);
            }
            end_clock(buffer,destFile);
            for (int i = arraySize; i < arraySize + number; i++){
                //fillBlock(arrayB[i],blockSize);
            }
            arraySize += number;
        }else if (strcmp(argv[i],"remove") == 0){
            int number = strtol(argv[i+1],NULL,10);
            char buffer[1000] = {0};
            sprintf(buffer,"remove of block %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                deleteBlockD(arraySize, number);
            }else{
                deleteBlockS(arraySize, number);
            }
            end_clock(buffer,destFile);
            arraySize -= number;
        }else if (strcmp(argv[i],"search_element") == 0){
            int index = strtol(argv[i+1],NULL,10);
            char buffer[1000] = {0};
            sprintf(buffer,"search of block %d %d ",arraySize,blockSize);
            start_clock();
            char* result = getBlockWithSimilarSum(arraySize,blockSize,index);
            end_clock(buffer,destFile);
        }
    }
    fclose(destFile);
    return 0;
}