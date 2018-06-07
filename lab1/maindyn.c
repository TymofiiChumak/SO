#include <math.h>
#include <sys/times.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> 
#include <dlfcn.h>

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
    sprintf(buffer,"Real Time: %6.3f, User Time: %6.3f, System Time: %6.3f\n",
        (en_time - st_time),
        (en_cpu.tms_utime - st_cpu.tms_utime),
        (en_cpu.tms_stime - st_cpu.tms_stime));
    fputs(buffer, destFile);
}




int main(int argc, char **argv){
    void *handle = dlopen("libcharBlocks.so", RTLD_LAZY);
    bool dynamic;
    void *obj = dlsym(handle,"arrayB");
    char** arrayB = (char**)obj;
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
                void (*creationArrayD)(int, int);
                creationArrayD = (void (*)(int, int))dlsym(handle,"creationArrayD"); 
                if (dlerror() == NULL) (*creationArrayD)(arraySize, blockSize);
            }else{
                void (*creationArrayS)(int, int);
                creationArrayS = (void (*)(int, int))dlsym(handle,"creationArrayS"); 
                if (dlerror() == NULL) (*creationArrayS)(arraySize, blockSize);
            }
            end_clock(buffer,destFile);
        }else if (strcmp(argv[i],"delete")==0){
            char buffer[1000] = {0};
            sprintf(buffer,"Deletion %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                void (*deletionArrayD)(int);
                deletionArrayD = (void (*)(int))dlsym(handle,"deletionArrayD"); 
                if (dlerror() == NULL) (*deletionArrayD)(arraySize);
            }else{
                void (*deletionArrayS)(int);
                deletionArrayS = (void (*)(int))dlsym(handle,"deletionArrayS"); 
                if (dlerror() == NULL) (*deletionArrayS)(arraySize);
            }
            end_clock(buffer,destFile);
        }else if (strcmp(argv[i],"add") == 0){
            int number = strtol(argv[i+1],NULL,10);
            char buffer[1000] = {0};
            sprintf(buffer,"creation of block %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                void (*addBlockD)(int, int, int);
                addBlockD = (void (*)(int, int, int))dlsym(handle,"addBlockD"); 
                if (dlerror() == NULL) (*addBlockD)(arraySize, blockSize, number);
            }else{
                void (*addBlockS)(int, int, int);
                addBlockS = (void (*)(int, int, int))dlsym(handle,"addBlockS"); 
                if (dlerror() == NULL) (*addBlockS)(arraySize, blockSize, number);
            }
            end_clock(buffer,destFile);
            for (int i = arraySize; i < arraySize + number; i++){
                void (*fillBlock)(char*, int);
                fillBlock = (void (*)(char*, int))dlsym(handle,"fillBlock"); 
                if (dlerror() == NULL) (*fillBlock)(arrayB[i], blockSize);
            }
            arraySize += number;
        }else if (strcmp(argv[i],"remove") == 0){
            int number = strtol(argv[i+1],NULL,10);
            char buffer[1000] = {0};
            sprintf(buffer,"remove of block %d %d ",arraySize,blockSize);
            start_clock();
            if (dynamic == true){
                void (*deleteBlockD)(int, int);
                deleteBlockD = (void (*)(int, int))dlsym(handle,"deleteBlockD"); 
                if (dlerror() == NULL) (*deleteBlockD)(arraySize, number);
            }else{
                void (*deleteBlockS)(int, int);
                deleteBlockS = (void (*)(int, int))dlsym(handle,"deleteBlockS"); 
                if (dlerror() == NULL) (*deleteBlockS)(arraySize, number);
            }
            end_clock(buffer,destFile);
            arraySize -= number;
        }else if (strcmp(argv[i],"search_element") == 0){
            int index = strtol(argv[i+1],NULL,10);
            char buffer[1000] = {0};
            sprintf(buffer,"search of block %d %d ",arraySize,blockSize);
            start_clock();
            char* (*getBlockWithSimilarSum)(int, int, int);
            getBlockWithSimilarSum = (char* (*)(int, int, int))dlsym(handle,"getBlockWithSimilarSum"); 
            char* result;
            if (dlerror() == NULL) result = (*getBlockWithSimilarSum)(arraySize,blockSize,index);
            end_clock(buffer,destFile);
        }
    }
    fclose(destFile);
    //dlclose(handle);
    return 0;
}