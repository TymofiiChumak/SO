#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv){
    void* handle = dlopen("libsource.so.1",RTLD_LAZY);
    printf("%s\n",dlerror());
    
    void (*fun1)(int, char*);
    fun1 = (void (*)(int,char*))dlsym(handle,"void_fun");
    
    int i = atoi(argv[1]); 
    char *s = argv[2];
    if (dlerror() == NULL) 
        fun1(i,s);
    int (*fun2)(int,char*);
    fun2 = (int (*)(int,char*))dlsym(handle,"int_fun");
    if (dlerror() == NULL){
        int res = fun2(atoi(argv[1]),argv[2]);
        printf("res = %d\n",res);
    }
    return 0;
}