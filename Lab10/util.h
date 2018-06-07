#include <time.h>

#define OP_ADD 0
#define OP_SUB 1
#define OP_MUL 2
#define OP_DIV 3

#define MSG_TYPE_ARTH 1
#define MSG_TYPE_ARTH_RESPONSE 2
#define MSG_TYPE_ADD 3
#define MSG_TYPE_ADD_RESPONSE 4
#define MSG_TYPE_REMOVE 5
#define MSG_TYPE_PING 6
#define MSG_TYPE_PING_RESPONSE 7

#define CLIENTS_NUM 20
#define CLIENT_NAME_LENGTH 128

struct msg_arth{
    int type;
    int a;
    int b;
    int op;
};

struct msg_response{
    int type;
    int result;
};

struct msg_client_name{
    int type;
    char client_name[CLIENT_NAME_LENGTH];
};

