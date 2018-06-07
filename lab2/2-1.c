#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void strmode(mode_t mode, char * buf) {
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++) {
        buf[i] = (mode & (1 << (8-i))) ? chars[i] : '-';
    }
    buf[9] = '\0';
}


void listDir(char* dirName,time_t t,char cmp){
    DIR* dir;
    if ((dir= opendir(dirName)) == NULL){
        //printf("Cannot open %s",dirName);
        return;
    }
    struct dirent* tmpfile;
    while ((tmpfile=readdir(dir)) != NULL){
        char fullName[1000] = {0};
        sprintf(fullName,"%s%s",dirName,tmpfile->d_name);
        struct stat tmpStat;
        if( stat(fullName,&tmpStat) < 0)
            return ;
        if (S_ISREG(tmpStat.st_mode)){
            time_t tmpTime = tmpStat.st_mtime;
            struct tm* tmp = localtime(&tmpTime);
            char timeBuffer[80] = {0};
            strftime(timeBuffer, 80, "%a, %d.%m.%Y %H:%M:%S", tmp);
            char modeBuffer[10] = {0};
            strmode(tmpStat.st_mode,modeBuffer);
            if ((cmp == 'e' && difftime(t,tmpTime) == 0) ||
                (cmp == 'g' && difftime(t,tmpTime) < 0)  ||
                (cmp == 'l' && difftime(t,tmpTime) > 0)){
                printf("%s---%d---%s---%s\n",
                    fullName,
                    (int)(tmpStat.st_size),
                    timeBuffer,
                    modeBuffer);
            }
        }else if (S_ISDIR(tmpStat.st_mode) != 0 && strcmp(tmpfile->d_name, ".") != 0  && strcmp(tmpfile->d_name, "..") != 0){
            sprintf(fullName,"%s/",fullName);
            listDir(fullName,t,cmp);
        } 
    }
    closedir(dir);
}


int main(int argc, char** argv){
    if (argc < 2) {
        printf("Too few args");
        return 0;
    }
    char* dirName = argv[1];
    char cmp = argv[2][0];
    
    struct tm tm;
    strptime(argv[3], "%Y,%d,%m,%H:%M:%S", &tm);
    time_t t = mktime(&tm);
    listDir(dirName,t,cmp);
    return 0;
}