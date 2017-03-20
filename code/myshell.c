#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


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


/* function prototypes go here... */
void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);

void Command_line_constructor(Command_line *cmd);
void redirection_argu_handle(Command_line *cmd, char *f_name);
void io_redirection_argu_handle(Command_line *cmd, char *in_f_name, char *out_f_name);
void input_redirection(Command_line *cmd);
void output_redirection(Command_line *cmd);
void io_redirection(Command_line *cmd);
void multi_pipe(Command_line **all_cmdline, const int all_cmdline_size);

void handle_sigchld(int sig);
int input_arg_handler(char *cmdline, Command_line **all_cmdline);
void create_child(char *time, char **argc, int argv);
void create_bg_process(char **argc);
void change_dir(char *arg_address);



/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
		
    // reference: http://www.microhowto.info/howto/reap_zombie_processes_using_a_sigchld_handler.html
    // sigchld handle register
	struct sigaction sa;
	sa.sa_handler = &handle_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, 0) == -1) {
	  perror(0);
	  exit(1);
	}


	while (1) 
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */
		
		process_cmd(cmdline);
	}
	return 0;
}

void Command_line_constructor(Command_line *cmd){
	int i;
	for(i = 0; i < MAX_ARGU_SIZE; i++) cmd->argc[i] = NULL;
	cmd->argv = 0;
	cmd->background = 0;
	cmd->input_redir = 0;
	cmd->output_redir = 0;
}

void redirection_argu_handle(Command_line *cmd, char *f_name){
	int index = 0;
	
		while(1){
			if(strcmp(cmd->argc[index], "<") == 0 || strcmp(cmd->argc[index], ">") == 0) break;
			index++;
		}
		strcpy(f_name, cmd->argc[index + 1]);

	while(cmd->argc[index] != NULL ){
		free(cmd->argc[index]);
		cmd->argv--;
		cmd->argc[index++] = NULL;
	}

}

void io_redirection_argu_handle(Command_line *cmd, char *in_f_name, char *out_f_name){
	int in_index = 0, out_index = 0;

	while(1){
		if(strcmp(cmd->argc[in_index], "<") == 0) break;
		in_index++;
		out_index++;
	}

	while(1){
		if(strcmp(cmd->argc[out_index], ">") == 0) break;
		out_index++;
	}

	strcpy(in_f_name, cmd->argc[in_index + 1]);
	strcpy(out_f_name, cmd->argc[out_index + 1]);

	while(cmd->argc[in_index] != NULL){
		free(cmd->argc[in_index]);
		cmd->argv--;
		cmd->argc[in_index++] = NULL;
	}
}

void input_redirection(Command_line *cmd){
	char f_name[MAX_CMDLINE_LEN];
	redirection_argu_handle(cmd, f_name);

	int input_fd = open(f_name, O_RDONLY);
	close(0);
	dup2(input_fd, 0);
	close(input_fd);
	if(execvp(cmd->argc[0], cmd->argc) == -1){
		_exit(1);	
	}
}

void output_redirection(Command_line *cmd){
	char f_name[MAX_CMDLINE_LEN];
	redirection_argu_handle(cmd, f_name);

	int output_fd = open(f_name, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
	close(1);
	dup2(output_fd, 1);
	close(output_fd);
	if(execvp(cmd->argc[0], cmd->argc) == -1){
		_exit(1);	
	}
}

void io_redirection(Command_line *cmd){
	char in_f_name[MAX_CMDLINE_LEN], out_f_name[MAX_CMDLINE_LEN];
	io_redirection_argu_handle(cmd, in_f_name, out_f_name);

	int pipefd[2];
	pipe(pipefd);
	pid_t pid = fork();

	if(pid == 0){
		int input_fd = open(in_f_name, O_RDONLY);
		close(0);
		close(1);
		dup2(input_fd, 0);
		dup2(pipefd[1], 1);
		close(input_fd);
		close(pipefd[0]);
		close(pipefd[1]);
		if(execvp(cmd->argc[0], cmd->argc) == -1){
			_exit(1);	
		}
	}else if(pid > 0){
		int status;
		int output_fd = open(out_f_name, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
		char buf;
		close(0);
		close(1);
		dup2(output_fd, 1);
		dup2(pipefd[0], 0);
		close(output_fd);
		close(pipefd[0]);
		close(pipefd[1]);
		waitpid(pid, &status, 0);

		if(status != 0) _exit(1);

		while(read(0, &buf, 1) != 0){
			write(1, &buf, 1);
		}

		_exit(0);
	}else{

	}
}

void multi_pipe(Command_line **all_cmdline, const int all_cmdline_size){
	int i, status = 0, pipefd[2];
	int prev_in_pipefd = -1, stdin_fd_copy = dup(0);
	pid_t pid;

	for(i = 0; i < all_cmdline_size; i++){
		pipe(pipefd);
		pid = fork();

		if(pid == 0){
			if(prev_in_pipefd != -1) dup2(prev_in_pipefd, 0);
			if(i != all_cmdline_size - 1){
				close(1);
				dup2(pipefd[1], 1);
				close(pipefd[0]);
			}else{
				close(pipefd[0]);
				close(pipefd[1]);
			}

			if(all_cmdline[i]->input_redir == 1 && all_cmdline[i]->output_redir == 0){
				input_redirection(all_cmdline[i]);
			}else if(all_cmdline[i]->input_redir == 0 && all_cmdline[i]->output_redir == 1){
				output_redirection(all_cmdline[i]);
			}else if(all_cmdline[i]->input_redir == 1 && all_cmdline[i]->output_redir == 1){
				io_redirection(all_cmdline[i]);
			}else{	
				if(execvp(all_cmdline[i]->argc[0], all_cmdline[i]->argc) == -1){
					_exit(1);
				}
			}
			
		}else if(pid > 0){
			close(0);
			dup2(pipefd[0], 0);
			close(pipefd[1]);

			if(all_cmdline[i]->background != 1) waitpid(pid, &status, 0);

			if(prev_in_pipefd != -1) close(prev_in_pipefd);	
		}else{

		}
		prev_in_pipefd = pipefd[0];

		if(status != 0){
			printf("%s: Command not found.\n", all_cmdline[i]->argc[0]);
			break;
		} 
	}
	
	close(prev_in_pipefd);
	close(0);
	dup2(stdin_fd_copy, 0);
	close(stdin_fd_copy);
}

//reference: http://www.microhowto.info/howto/reap_zombie_processes_using_a_sigchld_handler.html
void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
  errno = saved_errno;
}

int input_arg_handler(char *cmdline, Command_line **all_cmdline){
    char *token;
    int all_cmdline_size = 0;
    token = strtok(cmdline, " \t");

    Command_line *cmd = (Command_line *)malloc(sizeof(Command_line));
    Command_line_constructor(cmd);
    all_cmdline[all_cmdline_size++] = cmd;

    while(token != NULL){
        if(token[0] == '&'){
            cmd->background = 1;
        }else if(strcmp(token, "|") == 0){
        	cmd = (Command_line *)malloc(sizeof(Command_line));
        	Command_line_constructor(cmd);
        	all_cmdline[all_cmdline_size++] = cmd;
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

    return all_cmdline_size;
}

void create_child(char *time, char ** argc, int argv){
    int status;
    pid_t pid = fork();

    if(pid > 0){
        printf("child pid %d is started\n", pid);
        pid = waitpid(pid, &status, 0);
        printf("child pid %d is terminated with status %d\n", pid, status);
    }else if(pid == 0){
        sleep(atoi(time));

        int i;
        for(i = 0; i < argv; i++) free(argc[i]);

        _exit(0);
    }else{
        printf("Fail to create child\n");
    }

}

void create_bg_process(char **argc){
    pid_t pid = fork();

    if(pid > 0){
    	//empty
    }else if(pid == 0){
        int result = execvp(argc[0], argc);

        if(result < 0) printf("%s: Command not found.\n", argc[0]);

        _exit(0); 
    }else{
        printf("Fail to create child.\n");
    }
}

void change_dir(char *arg_address){
    int result = 0;

    if(arg_address == '\0'){
        result = chdir(getenv("HOME"));
    }else{
        result = chdir(arg_address);
    }

    if(result == -1) printf("Path not found\n"); 
}

void process_cmd(char *cmdline)
{
   	Command_line *all_cmdline[MAX_STRUCT_CMDLINE_SIZE] = {NULL};
	int i, all_cmdline_size = input_arg_handler(cmdline, all_cmdline);

	

    if(strcmp(all_cmdline[0]->argc[0], "exit") == 0){
    	for(i = 0; i < all_cmdline_size; i++){
		int j;
		for(j = 0; j < all_cmdline[i]->argv; j++){
			free(all_cmdline[i]->argc[j]);
		}
		free(all_cmdline[i]);
	}
        exit(0);
    }else if(strcmp(all_cmdline[0]->argc[0], "cd") == 0){
        change_dir(all_cmdline[0]->argc[1]);
    }else if(strcmp(all_cmdline[0]->argc[0], "child") == 0){
        create_child(all_cmdline[0]->argc[1], all_cmdline[0]->argc, all_cmdline[0]->argv);
    }else{
    	multi_pipe(all_cmdline, all_cmdline_size);
    }
 

	

    for(i = 0; i < all_cmdline_size; i++){
		int j;

		for(j = 0; j < all_cmdline[i]->argv; j++){

			free(all_cmdline[i]->argc[j]);
		}
		free(all_cmdline[i]);
		all_cmdline[i] = NULL;
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