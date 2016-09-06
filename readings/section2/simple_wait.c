#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "sys/types.h"
int main(void) {
    pid_t pid = fork();
    pid_t pr;
    int status;
    if (pid != 0) { // excute by parent process
        do{
            pr = waitpid(pid, &status,WNOHANG);
        }while(pr==0);
    }
    printf("Hello World\n: %d\n", pid);
    return 0;
}
