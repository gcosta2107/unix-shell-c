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