#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80

void check_conditions(char *user_input, int *validator, int *styleMode);

int check_cmd(char *cmd);

int check_history(char *cmd);

char* trim (char *string);

int find_char(char* args, char ch);

void split_allArgs(char *user_input, char **arguments, int *arg_count);

void parse_arg(char *cmd, char **parsed_cmd);

void clear_list(char **list);

void exec_cmd_seq(char **parsed_cmd);

void exec_cmd_par(char **parsed_cmd);

void split_pipe(char *user_input, char **arguments);

void split_send_redirect(char *user_input, char **arguments);

void exec_pipe(char* cmd);

void send_redirect(char* cmd);

int file_read(char *filename, char **arguments);

void batchMode(char *filename);

void printBatchCommands(char *filename);

int main(int argc, char *argv[]){

    int validator = 1;
    char user_input[MAX_LINE]; //linha inteira lida do teclado
    char *arguments[MAX_LINE/2 + 1]; //array de strings com separação dos comandos do input
    char *parsed_cmd[MAX_LINE/2 +1]; //temporariamente armazena o comando a ser executado
    char *input_copy[MAX_LINE/2 +1];
    char last_cmd[MAX_LINE/2 +1];
    int styleMode = 0;
    int arg_count = 0;

    last_cmd[0] = '\0';

    clear_list(arguments);
    clear_list(parsed_cmd);

    if(argc == 2){
        printBatchCommands(argv[1]);
        batchMode(argv[1]);
 
    }
    else if(argc > 2){
        printf("Insira menos argumentos\n");
    }
    else{
        while (validator) {
            if(styleMode == 0){
                printf("gcs seq> ");
                fflush(stdout);

                char *line = fgets(user_input, sizeof(user_input), stdin);
                fflush(stdin);
                
                //CTRL + D
                if(line == NULL){
                    printf("\n");
                    break;
                }

                check_conditions(user_input, &validator, &styleMode);

                split_allArgs(user_input, arguments, &arg_count);

                if(check_history(user_input) != 0){
                    for(int i = 0; i < arg_count; i++){
                        strcpy(last_cmd,arguments[i]);
                    }
                }

                for(int i = 0; i < arg_count; i++){
                    if(find_char(arguments[i],'|') == 0){
                        exec_pipe(arguments[i]);
                    }
                    else if(find_char(arguments[i],'>') == 0){
                        send_redirect(arguments[i]);
                    }
                    else if(check_history(arguments[i]) == 0){
                        if(last_cmd[0] == '\0'){
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
                        else if(check_cmd(arguments[i]) == 1){ //exit
                            validator = 0;
                        }
                        else if(check_cmd(arguments[i]) == 2){ //sequential
                            styleMode = 0;
                        }
                        else if(check_cmd(arguments[i]) == 3){ //parallel
                            styleMode = 1;
                        }
                    }
                }

                arg_count = 0;
                
            }
            
            else if(styleMode == 1){
                printf("gcs par> ");
                fflush(stdout);

                fgets(user_input, sizeof(user_input), stdin);
                fflush(stdin);

                check_conditions(user_input, &validator, &styleMode);

                split_allArgs(user_input, arguments, &arg_count);

                if(check_history(user_input) != 0){
                    for(int i = 0; i < arg_count; i++){
                        strcpy(last_cmd,arguments[i]);
                    }
                }

                for(int i = 0; i < arg_count; i++){
                    if(find_char(arguments[i],'|') == 0){
                        exec_pipe(arguments[i]);
                    }
                    else if(find_char(arguments[i],'>') == 0){
                        send_redirect(arguments[i]);
                    }
                    else if(check_history(arguments[i]) == 0){
                        if(last_cmd[0] == '\0'){
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
                            exec_cmd_par(input_copy);
                        }
                    }
                    else{
                        if(check_cmd(arguments[i]) == 0){
                            parse_arg(arguments[i], parsed_cmd);
                            exec_cmd_par(parsed_cmd);
                            clear_list(parsed_cmd);
                        }
                        else if(check_cmd(arguments[i]) == 1){ //exit
                            validator = 0;
                        }
                        else if(check_cmd(arguments[i]) == 2){ //sequential
                            styleMode = 0;
                        }
                        else if(check_cmd(arguments[i]) == 3){ //parallel
                            styleMode = 1;
                        }
                    }
                }
                
                for(int i = 0; i < arg_count; i++){
                    wait(NULL);
                }

                arg_count = 0;
            }
        }
    }

    return 0;
}