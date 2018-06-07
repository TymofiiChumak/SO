#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<sys/wait.h>

struct msg_buf {
  long mtype;
  char mtext[30];
} buffer1,buffer2,read_buffer1,read_buffer2;


int main(){
    int queue = msgget(IPC_PRIVATE, IPC_CREAT| 0666);
    buffer1.mtype = 1;
    sprintf(buffer1.mtext,"Witaj swiecie");
    buffer2.mtype = 1;
    sprintf(buffer2.mtext ,"Cala naprzod");
    msgsnd(queue, &buffer1, sizeof(buffer1.mtext), 0);
    msgsnd(queue, &buffer2, sizeof(buffer2.mtext), 0);
    read_buffer1.mtype = 1;
    read_buffer2.mtype = 1;
    msgrcv(queue, &read_buffer1, 30, 1, 0);
    msgrcv(queue, &read_buffer2, 30, 1, 0);
    printf("%s\n%s\n",read_buffer1.mtext,read_buffer2.mtext);
    msgctl(queue,IPC_RMID,NULL);
    return(0);

    

    

}