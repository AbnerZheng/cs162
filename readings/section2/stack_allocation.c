#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(void){
    int stuff = 7;
    pid_t pid = fork();
    printf("stuff is %d \n", stuff);
    if(pid == 0)
        stuff = 6;
    return 0;
}

