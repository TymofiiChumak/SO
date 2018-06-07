#include <sys/resource.h> 
#include <sys/time.h> 
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    struct rlimit rl;
    if (getrlimit(RLIMIT_CPU,&rl) != 0){
        printf("error while geting time limit");
        return EXIT_FAILURE;
    }
    printf("Time limit: Soft - %lld, Hard - %lld\n",(long long int)rl.rlim_cur,(long long int)rl.rlim_max);
    if (getrlimit(RLIMIT_DATA,&rl) != 0){
        printf("error while geting data limit");
        return EXIT_FAILURE;
    }
    printf("Data limit: Soft - %lld, Hard - %lld\n",(long long int)rl.rlim_cur,(long long int)rl.rlim_max); 
    while(1);  
    return 0;
}