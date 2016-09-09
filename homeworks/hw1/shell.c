#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <linux/limits.h>


#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

FILE* in;
FILE* out;



/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "print current working directory"},
    {cmd_cd, "cd", "change directory"}
};

int cmd_cd(struct tokens *tks){
    size_t tokens_len = tokens_get_length(tks);

    const char *path = tokens_get_token(tks, tokens_len-1);
    chdir(path);
    return 1;
}

int cmd_pwd(unused struct tokens *tokens){
    char pwd[80];
    getcwd(pwd, sizeof(pwd));
    fprintf(out,"%s\n", pwd);
    return 1;
}

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
    exit(0);
}


/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
            return i;
    return -1;
}
/* find cmd in path */
char* find_in_path(char* path, char* cmd){
    size_t path_len = strlen(path);
    if(path_len == 0){
        return NULL;
    }
    int length=0;
    while(length<path_len){
        if(path[length] == ':'){
            //find the cmd in the path
            break;
        }
        length++;
    }
    char* path_S = malloc(sizeof(char) *(length + strlen(cmd) + 2));
    strncpy(path_S, path, length);
    const char* d = "/";
    strcat(path_S, d);
    strcat(path_S, cmd);
    if( access( path_S, F_OK ) != -1 ) {
        return path_S;
    }
    path = path + length+1;
    return find_in_path(path, cmd);
}



/* Resolve path */
char *resolve(char* cmd){
    char* path = getenv("PATH");
    char* command = malloc(PATH_MAX);
    switch(cmd[0]){
        case '/':
        case '.':
            command = realpath(cmd, command);
            if(access(command,F_OK)!=-1)
                return command;
            else
                return NULL;
            break;
        default:
            break;
    }

    return find_in_path(path, cmd);
}

/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
         * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
         * foreground, we'll receive a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later. */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}
int main(unused int argc, unused char *argv[]) {
    init_shell();
    in = stdin;
    out = stdout;

    printf("current terminal controll pgid: %d\n", tcgetpgrp(0));
    printf("main process's pgid: %d,pid: %d\n", getpgid(getpid()), getpid());

    static char line[4096];
    int line_num = 0;

    signal(SIGTTOU,SIG_IGN);

    /* Please only print shell prompts when standard input is not a tty */
    if (shell_is_interactive)
        fprintf(stdout, "%d: ", line_num);

    while (fgets(line, 4096, stdin)) {
        /* Split our line into words. */
        struct tokens *tokens = tokenize(line);
        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        if (fundex >= 0) {
            size_t tokens_len = tokens_get_length(tokens);
            int wr=0;
            for(int i = 0; i<tokens_len; i++){
                char* token = tokens_get_token(tokens, i);
                char* file;
                switch(token[0]){
                    case '<':
                        file = tokens_get_token(tokens, ++i);
                        //dup_stdin =  dup(fileno(stdin));
                        in = fopen(file, "r");
                        fgets(line, 4096, in);
                        tokens = tokenize(line);
                        wr+=1;
                        break;
                    case '>':
                        file = tokens_get_token(tokens, ++i);
                        //dup_stdout = dup(fileno(stdout));
                        out = fopen(file, "w");
                        wr+=2;
                        break;
                    default:
                        break;
                }
            }
            cmd_table[fundex].fun(tokens);
            if(wr & 1){
                fclose(in);
                in = stdin;
            }
            if(wr & 2){
                fclose(out);
                out = stdout;
            }
        } else {
            /* REPLACE this to run commands as programs. */
            //fprintf(stdout, "This shell doesn't know how to run programs.\n");
            size_t tokens_len = tokens_get_length(tokens);
            char * cmd = tokens_get_token(tokens, 0);
            char * cmd_path =resolve(cmd);
            if(!cmd_path){
                fprintf(stdout, "There is no command named: %s\n", cmd);
            }else{
                pid_t pd = fork();
                int status;
                if(!pd){ // child process
                    int w_r =0;
                    setpgid(getpid(),getpid());
                    char *params[tokens_len+1];
                    int j=0;
                    for(int i = 0; i<tokens_len; i++){
                        char* token = tokens_get_token(tokens, i);
                        char* file;
                        switch(token[0]){
                            case '<':
                                w_r += 1;
                                file = tokens_get_token(tokens, ++i);
                                freopen(file, "r",stdin);
                                break;
                            case '>':
                                w_r += 2;
                                file = tokens_get_token(tokens, ++i);
                                freopen(file, "w", stdout);
                                break;
                            default:
                                params[j++] = tokens_get_token(tokens, i);
                                break;
                        }
                    }
                    params[j] = NULL;
                    execv(cmd_path, params);
                    if(w_r & 1){
                        fclose(stdin);
                    }
                    if(w_r & 2){
                        fclose(stdout);
                    }
                    exit(0);
                }else{
                    tcsetpgrp(STDIN_FILENO, pd);
                    wait(&status);
                    tcsetpgrp(STDIN_FILENO, getpid());
                }
            }
        }

        if (shell_is_interactive)
            /* Please only print shell prompts when standard input is not a tty */
            fprintf(stdout, "%d: ", ++line_num);

        /* Clean up memory */
        tokens_destroy(tokens);
    }

    return 0;
}

