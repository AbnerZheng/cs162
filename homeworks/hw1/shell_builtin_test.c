#include "stdio.h"

char line[100];
int main(){
    fgets(line, 100, stdin);
    printf("%s\n", line);
}

