#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMDLINE_LEN 256
#define MAX_DIR_LEN 256
#define MAX_STRUCT_CMDLINE_SIZE 16
#define MAX_ARGU_SIZE 16

typedef struct Command_line{
	char *argc[MAX_ARGU_SIZE];
	int argv;
	int background; /*0: no; 1: yes*/
	int input_redir; /*0: no; 1: yes*/
	int output_redir; /*0: no; 1: yes*/
} Command_line;


void print_argc(char **argc, int argv){
	int i;
	for(i = 0; i < argv; i++){
		printf("argc[%d]: %s\n", i, argc[i]);
	}
}

void print_command_line(Command_line *cmd){
	print_argc(cmd->argc, cmd->argv);
	printf("argv: %d\nbackgroup: %d\ninput_redir: %d\noutput_redir: %d\n", cmd->argv, cmd->background, cmd->input_redir, cmd->output_redir);
}

void print_all_command_line(Command_line **all_cmdline, int cmdline_size){
	int i;
	for(i = 0; i < cmdline_size; i++){
		printf("Command_line[%d]: ", i);
		print_command_line(all_cmdline[i]);
	}
}

void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);


int input_arg_handler(char *cmdline, Command_line **all_cmdline);
void Command_line_constructor(Command_line *cmd){
	int i;
	for(i = 0; i < MAX_ARGU_SIZE; i++) cmd->argc[i] = NULL;
	cmd->argv = 0;
	cmd->background = 0;
	cmd->input_redir = 0;
	cmd->output_redir = 0;
}

int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
	while (1) 
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */
		
		process_cmd(cmdline);
	}

	return 0;
}

int input_arg_handler(char *cmdline, Command_line **all_cmdline){
    char *token;
    int command_line_size = 0;
    token = strtok(cmdline, " \t");

    Command_line *cmd = (Command_line *)malloc(sizeof(Command_line));
    Command_line_constructor(cmd);
    all_cmdline[command_line_size++] = cmd;

    while(token != NULL){
        if(token[0] == '&'){
            cmd->background = 1;
        }else if(strcmp(token, "|") == 0){
        	cmd = (Command_line *)malloc(sizeof(Command_line));
        	all_cmdline[command_line_size++] = cmd;
        }else{
			if(strcmp(token, "<") == 0){
	        	cmd->input_redir = 1;
	        }else if(strcmp(token, ">") == 0){
	        	cmd->output_redir = 1;
	        }

            cmd->argc[cmd->argv] = (char *) malloc(sizeof(char) * (strlen(token) + 1));
			strcpy(cmd->argc[cmd->argv++], token);
        }
        token = strtok(NULL, " \t");
    }

    return command_line_size;
}

void process_cmd(char *cmdline)
{
   	Command_line *all_cmdline[MAX_STRUCT_CMDLINE_SIZE] = {NULL};
	int i, command_line_size = input_arg_handler(cmdline, all_cmdline);
   
	print_all_command_line(all_cmdline, command_line_size);

   	

	for(i = 0; i < command_line_size; i++){
		int j;
		for(j = 0; j < all_cmdline[i]->argv; j++){
			free(all_cmdline[i]->argc[j]);
		}
		free(all_cmdline[i]);
	}
	
}

void show_prompt() 
{   
    char buffer[MAX_DIR_LEN], *print_directory;
    getcwd(buffer, MAX_DIR_LEN);
    print_directory = strrchr(buffer, '/');
    
	printf("[%s] myshell> ", print_directory + 1);
}

int get_cmd_line(char *cmdline) 
{
    int i;
    int n;
    if (!fgets(cmdline, MAX_CMDLINE_LEN, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(cmdline);
    cmdline[--n] = '\0';
    i = 0;
    while (i < n && cmdline[i] == ' ')
    {
        ++i;
    }
    if (i == n)
    {
        // Empty command
        return -1;
    }
    return 0;
}