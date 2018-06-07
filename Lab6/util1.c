#define REPLY_SIZE 128
#define PROJ_ID 1
#define KEY_PATH "/home/tymofii"
#define MAX_MIRROR_SIZE 8192
#define MAX_MASSAGE_NUMBER 64
#define MSG_BEGIN 1
#define MSG_MIRROR 2
#define MSG_CALC 3
#define MSG_TIME 4
#define MSG_END 5
#define MSG_REPLY 6
#define MSG_IDENT 7
#define MSG_STOP 8
#define CALC_ADD 1
#define CALC_SUB 2
#define CALC_MUL 3
#define CALC_DIV 4
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



struct buf_msg_begin{
    int msgtype;
    pid_t pid;
    char queue_name[40];
};

struct buf_msg_mirror{
    int msgtype;
    int index;
    char text[MAX_MIRROR_SIZE];
};

struct buf_msg_calc{
    int msgtype;
    int type;
    int a;
    int b;
    int index;
};

struct buf_msg_time{
    int msgtype;
    int index;
};

struct buf_msg_end{
    int msgtype;
    int index;
};

struct buf_msg_reply{
    int msgtype;
    int rcv_msgtype;
    char text[REPLY_SIZE];
};

struct buf_msg_ident{
    int msgtype;
    int index;
};
int* parse_calc(char *line){
    char first[20];
    char second[20];
    char operator;
    int i = 0;
    int j = 0;
    while(line[i] != '+' && line[i] != '-' && line[i] != '*' && line[i] != '/' && line[i] != ' '){
        first[j] = line[i];
        j++;
        i++;
    }
    first[i] = 0;
    while(line[i] == ' ') i++;
    operator = line[i];
    i++;
    while(line[i] == ' ') i++;
    j = 0;
    while(line[i] != ' ' && line[i] != '\0' && (int)line[i] != 10){
        second[j] = line[i];
        j++;
        i++;
    }
    second[i] = 0;
    int *result = malloc(3*sizeof(int));
    result[0] = strtol(first,NULL,10);
    result[1] = strtol(second,NULL,10);
    switch (operator){
        case '+':
            result[2] = CALC_ADD;
            break;
        case '-':
            result[2] = CALC_SUB;
            break;
        case '*':
            result[2] = CALC_MUL;
            break;
        case '/':
            result[2] = CALC_DIV;
            break;
    }
    return result;
}
char *reverse(char *text){
    int len = strlen(text);
    char *res = malloc(sizeof(char) * (len+1));
    for (int i = 0 ; i < len ; i++){
        res[i] = text[len - i - 1];
    }
    return res;
}
