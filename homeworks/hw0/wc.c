#include "stdio.h"
#include "stdlib.h"

enum state_t {START_STATE, SPACE_STATE, NORMAL_STATE, LINE_STATE};

void wc(FILE* fp){
    int lines;
    int chars;
    int words;
    enum state_t cur = START_STATE;
    lines = chars = words = 0;
    char prev=0;
    char c;
    while ((c = fgetc(fp)) != EOF) { // standard C I/O file reading loop
        ++chars; //所有字符都要统计
        switch (cur){
            case START_STATE:
                switch(c){
                    case ' ':
                        cur = SPACE_STATE;
                        break;
                    case '\n':
                        ++lines;
                        cur = LINE_STATE;
                        break;
                    default:
                        ++words;
                        cur = NORMAL_STATE;
                }
                break;
            case SPACE_STATE:
                switch(c){
                    case ' ':
                        break;
                    case '\n':
                        lines ++;
                        cur = LINE_STATE;
                        break;
                    default:
                        ++words;
                        cur = NORMAL_STATE;
                }
                break;
            case NORMAL_STATE:
                switch(c){
                    case ' ':
                        cur = SPACE_STATE;
                        break;
                    case '\n':
                        cur = LINE_STATE;
                        lines++;
                        break;
                    default:
                        break;
                }
                break;
            case LINE_STATE:
                switch(c){
                    case ' ':
                        cur = SPACE_STATE;
                        break;
                    case '\n':
                        ++lines;
                        break;
                    default:
                        ++words;
                        cur = NORMAL_STATE;
                }
                break;
        }
    }
    printf("                 %d  %d  %d\n", lines, words, chars);
    fclose(fp);
}
int main(int argc, char *argv[]) {
    FILE *fp;
    if(argc == 1){
        fp = stdin;
    }else if(argc ==2){
        fp = fopen(argv[1], "r");
        if(!fp) {
            perror("File opening failed");
            return EXIT_FAILURE;
        }
    }else{
        printf("usage error\n");
        return 0;
    }
    wc(fp);
    return 0;
}
