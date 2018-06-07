#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

char cmp;
time_t t;

void strmode(mode_t mode, char * buf) {
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++) {
        buf[i] = (mode & (1 << (8-i))) ? chars[i] : '-';
    }
    buf[9] = '\0';
}

int processFile(const char *file, const struct stat *tmpStat,
                int flag, struct FTW *s){
    if (s->level >1) return 0;
    if (flag == FTW_F){
        time_t tmpTime = tmpStat->st_mtime;
        struct tm* tmp = localtime(&tmpTime);
        char timeBuffer[80];
        strftime(timeBuffer, 80, "%a, %d.%m.%Y %H:%M:%S", tmp);
        char modeBuffer[10] = {0};
        strmode(tmpStat->st_mode,modeBuffer);
        if ((cmp == 'e' && difftime(t,tmpTime) == 0) ||
            (cmp == 'g' && difftime(t,tmpTime) < 0)  ||
            (cmp == 'l' && difftime(t,tmpTime) > 0)){
            printf("%s---%d---%s---%s\n",
                   file,
                   (int)(tmpStat->st_size),
                   timeBuffer,
                   modeBuffer);
        }
    }
    return 0;
}


int main(int argc, char** argv){
    if (argc < 2) {
        printf("Too few args");
        return 0;
    }
    char* dirName = argv[1];
    cmp = argv[2][0];
    struct tm tm;
    strptime(argv[3], "%Y,%d,%m,%H:%M:%S", &tm);
    t = mktime(&tm);
    int flags = FTW_CHDIR | FTW_DEPTH | FTW_MOUNT;
    nftw(dirName,processFile,1,0);
    return 0;
}