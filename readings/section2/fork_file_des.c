#include "stdio.h"
#include "stdlib.h"
//#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"

int main(void){
    int fd;
    fd = open("output.txt", O_CREAT|O_TRUNC|O_WRONLY, 0666);

    if(!fork()){
        write(fd, "hello ", 6);
        exit(0);
    }else{
        int status;

        wait(&status);
        write(fd, "world\n", 6);
    }
    return 0;
}
