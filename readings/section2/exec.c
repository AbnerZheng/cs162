#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/types.h"
//#include "types"

int main(void){
    char** argv = (char**) malloc(3*sizeof(char*));
    argv[0] = "/bin/ls";
    argv[1] = ".";
    argv[2] = NULL;
    for(int i = 0; i< 10; i++){
        printf("%d\n", i);
        if(i == 3){
            pid_t pi = fork();
            if(pi == 0){
                execv("/bin/ls", argv);
            }
        }
    }
    return 0;
}
