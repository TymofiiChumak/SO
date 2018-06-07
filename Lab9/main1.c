#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define PRINT_LESS 1
#define PRINT_ALL 0
#define SEARCH_LESS 0
#define SEARCH_EQUAL 1
#define SEARCH_ABOVE 2
#define LINE_SIZE 2048

int manufacturer_thread_num;
int consumer_thread_num;
int buffer_size;
int line_size;
int search_type;
int print_type;
int wait_time;
FILE *source;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
int file_empty;
int line_num = 0;



struct ceil{
    char *text;
    int line_index;
    pthread_cond_t is_empty;
    pthread_cond_t is_not_empty;
    pthread_mutex_t ceil_mutex;
};

struct circle_buffer{
    struct ceil *buffer;
    pthread_mutex_t buffer_info_mutex;
    pthread_cond_t cond_owerflow;
    pthread_cond_t cond_empty;
    int last_push;
    int last_pop;
    int size;
};


struct circle_buffer buffer;

char* buffer_dump(){
    char *buf = malloc(1000);
    for (int i = 0; i < buffer_size; i++){        
        if (buffer.buffer[i].text == NULL){
            sprintf(buf,"%s #%d [empty]",buf,i);
        }else{
            sprintf(buf,"%s #%d [%s]",buf,i,buffer.buffer[i].text);
        }
    }
    return buf;
}

void add_to_buffer(char *text_buffer, int line_index){

    pthread_mutex_lock(&buffer.buffer_info_mutex);
    
    while (buffer.size >= buffer_size){
        pthread_cond_wait(&buffer.cond_owerflow, &buffer.buffer_info_mutex);
    }
    buffer.size++;   
    buffer.last_push++;    
    if (buffer.last_push >= buffer_size) buffer.last_push = 0;

    int index = buffer.last_push;
    pthread_mutex_unlock(&buffer.buffer_info_mutex);

    pthread_mutex_lock(&buffer.buffer[index].ceil_mutex);    
    while (buffer.buffer[index].text != NULL){
        pthread_cond_wait(&buffer.buffer[index].is_empty,&buffer.buffer[index].ceil_mutex);
    }
    buffer.buffer[index].text = text_buffer;
    buffer.buffer[index].line_index = line_index;
    //printf("add index: %d text: %s %s\n", index,text_buffer,buffer_dump());
    if (print_type == PRINT_ALL) printf("add index: %d text: %s\n",index,text_buffer);
    pthread_cond_broadcast(&buffer.buffer[index].is_not_empty);
    pthread_mutex_unlock(&buffer.buffer[index].ceil_mutex);  

    pthread_mutex_lock(&buffer.buffer_info_mutex);
    pthread_cond_broadcast(&buffer.cond_empty);     
    pthread_mutex_unlock(&buffer.buffer_info_mutex);   
}    

int get_from_buffer(char **text){
    
    pthread_mutex_lock(&buffer.buffer_info_mutex);
    while (buffer.size < 1){
        pthread_cond_wait(&buffer.cond_empty, &buffer.buffer_info_mutex);
    }    
    buffer.size--;
    buffer.last_pop++;
    if (buffer.last_pop >= buffer_size) buffer.last_pop = 0;    
    int index = buffer.last_pop;
    pthread_mutex_unlock(&buffer.buffer_info_mutex);       
    pthread_mutex_lock(&buffer.buffer[index].ceil_mutex);
    while (buffer.buffer[index].text == NULL){
        pthread_cond_wait(&buffer.buffer[index].is_not_empty,&buffer.buffer[index].ceil_mutex);
    }
    *text = buffer.buffer[index].text;
    int line_index = buffer.buffer[index].line_index;
    buffer.buffer[index].text = NULL;
    if (print_type == PRINT_ALL) printf("get index: %d, text: %s\n", index, text);
    pthread_cond_broadcast(&buffer.buffer[index].is_empty);
    pthread_mutex_unlock(&buffer.buffer[index].ceil_mutex);

    pthread_mutex_lock(&buffer.buffer_info_mutex);       
    pthread_cond_broadcast(&buffer.cond_owerflow);        
    pthread_mutex_unlock(&buffer.buffer_info_mutex);           

    return line_index;
}

void *manufacturer(void *args){
    int num = ((int*)args)[0];
    char *text_buffer;
    if (wait_time != 0){
        alarm(wait_time);
    }
    while(1){
        text_buffer = malloc(LINE_SIZE * sizeof(char));        
        pthread_mutex_lock(&file_mutex);
        if (fscanf(source,"%[^\n]\n",text_buffer) == EOF){
            file_empty--;
            if (file_empty == 0 && buffer.size == 0) raise(SIGUSR1);
            pthread_mutex_unlock(&file_mutex);
            if (wait_time == 0) return 0;            
            continue;
        }
        int line_index = line_num;
        line_num++;
        pthread_mutex_unlock(&file_mutex);
        if (print_type == PRINT_ALL) printf("readed: %s\n",text_buffer);
        add_to_buffer(text_buffer,line_index);
    }
}

void *consumer(void *args){
    int num = ((int*)args)[0];
    if (wait_time != 0){
        alarm(wait_time);
    }
    while(1){
        pthread_mutex_lock(&buffer.buffer_info_mutex);  
        if (wait_time == 0) {
            if (file_empty == 0 && buffer.size == 0) raise(SIGUSR1);
        }
        pthread_mutex_unlock(&buffer.buffer_info_mutex);  
        char *text;
        int index = get_from_buffer(&text); 
        switch(search_type){
            case SEARCH_LESS:
                if (strlen(text) < line_size) printf("index %d text %s\n", index, text);   
                break;
            case SEARCH_EQUAL:
                if (strlen(text) == line_size) printf("index %d text %s\n", index, text);   
                break;
            case SEARCH_ABOVE:
                if (strlen(text) > line_size) printf("index %d text %s\n", index, text);   
                break;
        }
        free(text);
            
    }
}

void signal_handler(int signo){
    if(signo == SIGALRM){
        printf("time end\n");
    }else if (signo == SIGUSR1){
        printf("program end\n");
    }else if (signo == SIGINT){
        printf("received interrupt signal\n");
    }
    fclose(source);
    exit(0);
}

int main(int argc, char **argv){
    if (argc < 1){
        perror("Too few args");
        exit(EXIT_FAILURE);
    }
    FILE *conf = fopen(argv[1],"r");
    if (conf == NULL){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    char *source_name = malloc(128 * sizeof(char));
    fscanf(conf,"%d",&manufacturer_thread_num);
    fscanf(conf,"%d",&consumer_thread_num);
    fscanf(conf,"%d",&buffer_size);
    fscanf(conf,"\n%[^\n]\n",source_name);
    fscanf(conf,"%d",&line_size);
    fscanf(conf,"%d",&search_type);
    fscanf(conf,"%d",&print_type);
    fscanf(conf,"%d",&wait_time);
    fclose(conf);
    source = fopen(source_name,"r");
    if (source == NULL){
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    buffer.buffer = malloc(sizeof(struct ceil) * buffer_size);
    for (int i = 0; i < buffer_size; i++){
        buffer.buffer[i].text = NULL;
    }
    buffer.size = 0;
    buffer.size = 0;
    buffer.last_pop = -1;
    buffer.last_push = -1;
    file_empty = manufacturer_thread_num;
    
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask,SIGUSR1);
    sa.sa_handler = signal_handler;
    sigaction(SIGALRM,&sa,NULL);
    sigaction(SIGUSR1,&sa,NULL);
    sigaction(SIGINT,&sa,NULL);

    pthread_cond_init(&buffer.cond_owerflow,NULL);
    pthread_cond_init(&buffer.cond_empty,NULL);
    for (int i = 0; i < buffer_size; i++){
        pthread_mutex_init(&buffer.buffer[i].ceil_mutex,NULL);
        pthread_cond_init(&buffer.buffer[i].is_not_empty,NULL);
        pthread_cond_init(&buffer.buffer[i].is_empty,NULL);
    }
    pthread_t manufacturers[manufacturer_thread_num];
    for (int i = 0; i < manufacturer_thread_num; i++){
        pthread_create(&manufacturers[i],NULL,manufacturer,&i);
    }
    pthread_t consumers[consumer_thread_num];
    for (int i = 0; i < consumer_thread_num; i++){
        pthread_create(&consumers[i],NULL,consumer,&i);
    }
    void *ret_val;
    for (int i = 0; i < manufacturer_thread_num; i++){
        pthread_join(manufacturers[i],NULL);
    }
    for (int i = 0; i < consumer_thread_num; i++){
        pthread_join(consumers[i],NULL);
    }
    fclose(source);

}