#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "functions.h"

#define MAX_LINE 80

void check_conditions(char *user_input, int *validator, int *styleMode)
{
    if(strncmp(user_input,"exit",strlen("exit")) == 0){
        *validator = 0;
    }
    else if(strncmp(user_input, "style sequential",strlen("style sequential")) == 0){
        *styleMode = 0;
    }
    else if (strncmp(user_input,"style parallel",strlen("style parallel")) == 0){
        *styleMode = 1;
    }
}

int check_cmd(char *cmd)
{
    if(strncmp(cmd,"exit",strlen("exit")) == 0){
        return 1;
    }
    else if(strncmp(cmd, "style sequential",strlen("style sequential")) == 0){
        return 2;
    }
    else if (strncmp(cmd,"style parallel",strlen("style parallel")) == 0){
        return 3;
    }
    else{
        return 0;
    }
}

int check_history(char *cmd)
{
    if(strncmp(cmd,"!!",strlen("!!")) == 0){
        return 0;
    }
    else{
        return 1;
    }
}

char* trim (char *string)
{
    int i;

    while (isspace (*string)){
        string++;
    }

    for (i = strlen(string) - 1; isspace(string[i]); i--);

    string[i + 1] = '\0';

    return string;
}

int find_char(char* args, char ch)
{
    char* ptr;
 
    ptr = strchr(args, ch);

    if(ptr == NULL){
        return 1;
    }
    else{
        return 0;
    }
    
}

void clear_list(char **list)
{
    int size = sizeof list / sizeof list[0];

    for(int i = 0; i < size+1;i++){
        list[i] = NULL;
    }
}

void split_allArgs(char *user_input, char **arguments, int *arg_count)
{
    char *separator;

    separator = strtok(user_input, ";");
    
    while( separator != NULL) {
        arguments[*arg_count] = strdup(separator);
        separator = strtok(NULL, ";");
        (*arg_count)++;
    }
}

void parse_arg(char *cmd, char **parsed_cmd)
{
    char *separator;
    int arg_count = 0;
    char *temp;

    temp = trim(cmd);

    int size = sizeof temp / sizeof temp[0];

    separator = strtok(temp, " ");
    
    while( separator != NULL) {
        if(arg_count != size){
            parsed_cmd[arg_count] = malloc(size * sizeof(char));
		    parsed_cmd[arg_count] = strdup(separator);
            separator = strtok(NULL, " ");
            arg_count++;
        }
    }

    parsed_cmd[arg_count] = NULL;
}

void exec_cmd_seq(char **parsed_cmd)
{
    pid_t pid;

    pid = fork();

    if(pid < 0){
        fprintf(stderr, "Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        if(execvp(parsed_cmd[0],parsed_cmd) == -1){
            printf("Invalid command\n");
        }
        exit(EXIT_SUCCESS);
    }
    else{
        wait(NULL);
    }
}

void exec_cmd_par(char **parsed_cmd)
{
    pid_t pid;

    pid = fork();

    if(pid < 0){
        fprintf(stderr, "Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        if(execvp(parsed_cmd[0],parsed_cmd) == -1){
            printf("Invalid command\n");
        }
    }
}

void split_pipe(char *user_input, char **arguments)
{
    char *separator;
    int i = 0;

    separator = strtok(user_input, "|");
    
    while( separator != NULL) {
        arguments[i] = strdup(separator);
        separator = strtok(NULL, "|");
        i++;
    }
}

void exec_pipe(char* cmd)
{
    char *pipe_args[MAX_LINE/2 +1];
    char *buffer1[MAX_LINE/2 +1];
    char *buffer2[MAX_LINE/2 +1];

    split_pipe(cmd,pipe_args);

    parse_arg(pipe_args[0], buffer1);
    
    parse_arg(pipe_args[1], buffer2);

    int fd[2];

    // Creating pipes
	if (pipe(fd) == -1)
	{
		printf("Error: pipe failed");
		fflush(stdout);
	}

	pid_t pid1 = fork();
	
	if (pid1 < 0)
	{
		printf("Error creating child");
		fflush(stdout);
        exit(EXIT_FAILURE);
	}
    else if (pid1 == 0) 
	{   
        close(fd[0]);
        dup2(fd[1],STDOUT_FILENO);
        execvp(buffer1[0],buffer1);
        exit(0);
    }

    pid_t pid2 = fork();

    if (pid2 < 0)
	{
		printf("Error creating child");
		fflush(stdout);
        exit(EXIT_FAILURE);
	}
    else if (pid2 == 0) 
	{
        close(fd[1]);
        dup2(fd[0],STDIN_FILENO);
        execvp(buffer2[0],buffer2);
        exit(0);
	}

    close(fd[0]);
    close(fd[1]);
    if (pid1 >= 0) {
        wait(NULL);
        if (pid2 >= 0) {
            wait(NULL);
        }
    }
	

}

void split_send_redirect(char *user_input, char **arguments)
{
    char *separator;
    int i = 0;

    separator = strtok(user_input, ">");
    
    while( separator != NULL) {
        arguments[i] = strdup(separator);
        separator = strtok(NULL, ">");
        i++;
    }
}

void send_redirect(char* cmd)
{
    char *args[MAX_LINE/2 +1];
    char *buffer[MAX_LINE/2 +1];

    split_send_redirect(cmd,args); 

    parse_arg(args[0], buffer);

    int file;
    pid_t pid;
    int input;

    pid = fork();

    if(pid < 0){
        fprintf(stderr, "Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        file = open(args[1], O_CREAT | O_WRONLY, 0777);
	    dup2(file, STDOUT_FILENO);

        input = execvp(buffer[0],buffer);
        dup2(input, STDIN_FILENO);

        close(file);
        exit(0);
    }

    if (pid >= 0) {
        wait(NULL);
    }
}

void split_batch(char *line, char **arguments)
{
    char *separator;
    int i = 0;

    separator = strtok(line, "\n");
    
    while( separator != NULL) {
        arguments[i] = strdup(separator);
        separator = strtok(NULL, "\n");
        i++;
    }
}

void exec_cmd_batch(char **parsed_cmd, char *readline)
{
    pid_t pid;

    pid = fork();

    if(pid < 0){
        fprintf(stderr, "Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        if(execvp(parsed_cmd[0],parsed_cmd) == -1){
            printf("Invalid command\n");
        }
        exit(EXIT_SUCCESS);
    }
    else{
        wait(NULL);
    }
}

void batchMode(char *filename)
{
    char readline[MAX_LINE];
    char *arguments[MAX_LINE/2 + 1];
    char *parsed_cmd[MAX_LINE/2 + 1];
    char *input_copy[MAX_LINE/2 + 1]; 
    char last_cmd[MAX_LINE/2 +1];

    last_cmd[0] = '\0';

    FILE *fp = fopen (filename, "r");

    while (fgets(readline, sizeof(readline), fp)){

        int i = 0;

        strcpy(last_cmd, readline);

        printf("%s", readline);

        split_batch(readline, arguments);
        
        if(strncmp(readline,"exit",sizeof("exit")) != 0){
            if(find_char(arguments[i],'|') == 0){
                exec_pipe(arguments[i]);
            }
            else if(find_char(arguments[i],'>') == 0){
                send_redirect(arguments[i]);
            }
            else if(check_history(arguments[i]) == 0){
                if(i = 0){
                    printf("No commands\n");
                }
                else if(find_char(last_cmd,'|') == 0){
                    exec_pipe(last_cmd);
                }
                else if(find_char(last_cmd,'>') == 0){
                    send_redirect(last_cmd);
                }
                else{
                    parse_arg(last_cmd,input_copy);
                    exec_cmd_seq(input_copy);
                }
            }
            else{
                if(check_cmd(arguments[i]) == 0){
                    parse_arg(arguments[i], parsed_cmd);
                    exec_cmd_seq(parsed_cmd);
                    clear_list(parsed_cmd);
                }
            }
        }
        else{
            break;
        }
    }
    fclose(fp);
}