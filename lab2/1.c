#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/times.h>
#include <stdlib.h>
#include <fcntl.h>
#include <zconf.h>

clock_t st_time;
clock_t en_time;
struct tms st_cpu;
struct tms en_cpu;

void start_clock() {
    st_time = times(&st_cpu);
}

void end_clock(char *msg, FILE *destFile)
{
    en_time = times(&en_cpu);
    fputs(msg,destFile);
    char buffer[1000] = {0};
    sprintf(buffer,"Real Time: %d, User Time: %d, System Time: %d\n",
            (en_time - st_time),
            (en_cpu.tms_utime - st_cpu.tms_utime),
            (en_cpu.tms_stime - st_cpu.tms_stime));
    fputs(buffer, destFile);
}




int main(int argc, char** argv){

    if (strcmp(argv[1],"generate") == 0){
        if (argc < 5 ){
            printf("Too few arguments\n");
            return 0;
        }
        char *fileName = argv[2];
        int block_number = (int)strtol(argv[3],NULL,10);
        if (errno != 0){
            printf("Wrong block number: %s\n",argv[3]);
            return 0;
        }

        int block_size = (int)strtol(argv[4],NULL,10);
        {
            if (errno != 0){
                printf("Wrong block size: %s\n",argv[4]);
                return 0;
            }
        }
        srand(time(NULL));
        char* buffer = calloc(block_size, sizeof(char));
        FILE* time_result = fopen("wyniki.txt","a");
        char text_buffer[100] = {0};
        if (strcmp(argv[5],"lib") == 0) {
            FILE *destFile = fopen(fileName,"w");
            if (!destFile){
                printf("File opening filed\n");
                return 0;
            }
            sprintf(text_buffer, "generating using lib functions %s\n   number of blocks: %d, size of block: %d\n   ",
                    fileName, block_number, block_size);
            start_clock();
            for (int i = 0; i < block_number; i++) {
                for (int i = 0; i < block_size; i++) {
                    buffer[i] = (char) (rand() % 26 + 65);
                }
                fwrite(buffer, sizeof(char), block_size, destFile);
            }
            end_clock(text_buffer, time_result);
            fclose(destFile);
        }else if (strcmp(argv[5],"sys") == 0){
            int destFile = open(fileName,O_WRONLY | O_CREAT,S_IRUSR|S_IWUSR);
            if (!destFile){
                printf("File opening filed\n");
                return 0;
            }
            sprintf(text_buffer, "generating using sys functions %s\n   number of blocks: %d, size of block: %d\n   ",
                    fileName, block_number, block_size);
            start_clock();
            for (int i = 0; i < block_number; i++) {
                for (int i = 0; i < block_size; i++) {
                    buffer[i] = (char) (rand() % 26 + 65);
                }
                write(destFile,buffer,block_size);
            }
            end_clock(text_buffer, time_result);
            close(destFile);
        }
        fclose(time_result);
    }else if(strcmp(argv[1],"sort") == 0) {
        if (argc < 5) {
            printf("Too few arguments\n");
            return 0;
        }
        char *fileName = argv[2];
        int block_number = (int) strtol(argv[3], NULL, 10);
        if (errno != 0) {
            printf("Wrong block number: %s\n", argv[3]);
            return 0;
        }
        int block_size = (int) strtol(argv[4], NULL, 10);
        {
            if (errno != 0) {
                printf("Wrong block size: %s\n", argv[4]);
                return 0;
            }
        }
        char *buffer1 = calloc(block_size, sizeof(char));
        char *buffer2 = calloc(block_size, sizeof(char));
        FILE *time_result = fopen("wyniki.txt", "a");
        char text_buffer[100] = {0};
        if (strcmp(argv[5], "lib") == 0) {
            sprintf(text_buffer, "Sorting %s using lib functions\n   number of blocks: %d, size of block: %d\n   ",
                    fileName, block_number, block_size);
            FILE *destFile = fopen(fileName,"r+");
            if (!destFile) {
                printf("File opening filed\n");
                return 0;
            }
            start_clock();
            for (int i = 1; i < block_number; i++) {
                int j = i - 1;
                fseek(destFile,sizeof(char)*i*block_size,SEEK_SET);
                fread(buffer1, sizeof(char),block_size,destFile);
                fseek(destFile,sizeof(char)*j*block_size,SEEK_SET);
                fread(buffer2, sizeof(char),block_size,destFile);
                while (j >= 0 && buffer1[0] < buffer2[0]) {
                    fseek(destFile,sizeof(char)*(j+1)*block_size,SEEK_SET);
                    fwrite(buffer2, sizeof(char),block_size,destFile);
                    j -= 1;
                    fseek(destFile,sizeof(char)*j*block_size,SEEK_SET);
                    fread(buffer2, sizeof(char),block_size,destFile);
                }
                fseek(destFile,sizeof(char)*(j+1)*block_size,SEEK_SET);
                fwrite(buffer1, sizeof(char),block_size,destFile);
            }
            end_clock(text_buffer, time_result);
            fclose(destFile);
        }else  if (strcmp(argv[5], "sys") == 0) {
            sprintf(text_buffer, "Sorting %s using sys functions\n   number of blocks: %d, size of block: %d\n   ",
                    fileName, block_number, block_size);
            int destFile = open(fileName,O_RDWR | O_CREAT,S_IRUSR|S_IWUSR);
            if (!destFile) {
                printf("File opening filed\n");
                return 0;
            }
            start_clock();
            for (int i = 1; i < block_number; i++) {
                int j = i - 1;
                lseek(destFile,i*block_size,SEEK_SET);
                read(destFile,buffer1,block_size);
                lseek(destFile,j*block_size,SEEK_SET);
                read(destFile,buffer2,block_size);
                while (j >= 0 && buffer1[0] < buffer2[0]) {
                    lseek(destFile,(j+1)*block_size,SEEK_SET);
                    write(destFile,buffer2,block_size);
                    j -= 1;
                    lseek(destFile,j*block_size,SEEK_SET);
                    read(destFile,buffer2,block_size);
                }
                lseek(destFile,(j+1)*block_size,SEEK_SET);
                write(destFile,buffer1,block_size);
            }
            end_clock(text_buffer, time_result);
            close(destFile);
        }
        fclose(time_result);

    }else if(strcmp(argv[1],"copy") == 0){
        if (argc < 7 ){
            printf("Too few arguments\n");
            return 0;
        }
        char *fileName1 = argv[2];
        char *fileName2 = argv[3];
        int block_number = (int)strtol(argv[4],NULL,10);
        {
            if (errno != 0){
                printf("Wrong block number: %s\n",argv[4]);
                return 0;
            }
        }
        int block_size = (int)strtol(argv[5],NULL,10);
        {
            if (errno != 0){
                printf("Wrong block size: %s\n",argv[5]);
                return 0;
            }
        }
        char* buffer = calloc(block_size, sizeof(char));
        char text_buffer[100] = {0};
        FILE *time_result = fopen("wyniki.txt", "a");
        if (strcmp(argv[6],"lib") == 0) {
            sprintf(text_buffer, "Copying from %s to %s using lib functions\n   number of blocks: %d, size of block: %d\n   ",
                    fileName1, fileName2, block_number, block_size);
            FILE *sourceFile = fopen(fileName1,"r");
            if (!sourceFile){
                printf("Source file opening filed\n");
                return 0;
            }
            FILE *destFile = fopen(fileName2,"w");
            if (!destFile){
                printf("Destination file opening filed\n");
                return 0;
            }
            start_clock();
            for (int i = 0; i < block_number; i++) {
                size_t ret_code = fread(buffer, sizeof(char), block_size, sourceFile);
                if (ret_code != block_size) {
                    if (feof(sourceFile))
                        printf("Error reading %s: unexpected end of file\n", fileName1);
                    else if (ferror(sourceFile)) {
                        perror("Error reading");
                    }
                }
                fwrite(buffer, sizeof(char), block_size, destFile);
            }
            end_clock(text_buffer, time_result);
            fclose(sourceFile);
            fclose(destFile);
        }else if (strcmp(argv[6],"sys") == 0){
            sprintf(text_buffer,"Copying from %s to %s using sys functions\n   number of blocks: %d, size of block: %d\n   ",
                    fileName1,fileName2,block_number,block_size);
            int sourceFile = open(fileName1,O_RDONLY | O_CREAT,S_IRUSR|S_IWUSR);
            if (!sourceFile){
                printf("Source file opening filed\n");
                return 0;
            }
            int destFile = open(fileName2,O_WRONLY | O_CREAT,S_IRUSR|S_IWUSR);
            if (!destFile){
                printf("Destination file opening filed\n");
                return 0;
            }
            start_clock();
            for (int i = 0 ; i < block_number; i++){
                read(sourceFile, buffer, block_size);
                write(destFile, buffer,block_size);
            }
            end_clock(text_buffer,time_result);
            close(sourceFile);
            close(destFile);
        }
        fclose(time_result);
    }else{
        printf("Unknown argument: %s\n",argv[1]);
        return 0;
    }
    return 0;
}

