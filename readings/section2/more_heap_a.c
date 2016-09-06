#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
void printTenNumbers(int *arr) {
    int i;
    printf("\n");
    for(i=0;i<10;i++) {
        printf("%d",arr[i]);
    }
    exit(0);

}

int main() {
    int *arr, i;
    arr = (int *) malloc (sizeof(int));

    arr[0] = 0;
    for(i=1;i<10;i++) {
        arr = (int *) realloc( arr, (i+1) * sizeof(int));
        arr[i] = i;
        if (i == 7) {
            pid_t pid = fork();
            if (pid == 0) {
                printTenNumbers(arr); }
        }

}
printTenNumbers(arr); return 0;

}
