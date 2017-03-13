#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_CMDLINE_LEN 256
#define MAX_DIR_LEN 256


/* function prototypes go here... */
void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);

void handle_sigchld(int sig);
int input_arg_handler(char *cmdline, char **argc, char *background);
void create_child(char *time, char **argc, int argv);
void create_linux_program_child(char **argc, char *background);
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

//reference: http://www.microhowto.info/howto/reap_zombie_processes_using_a_sigchld_handler.html
void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
  errno = saved_errno;
}

int input_arg_handler(char *cmdline, char **argc, char *background){
    char *token;
    int argv = 0;
    token = strtok(cmdline, " \t");

    while(token != NULL){
        if(token[0] == '&'){
            *background = '&';
        }else{
            argc[argv] = (char *) malloc(sizeof(char) * (strlen(token) + 1));
			strcpy(argc[argv++], token);
        }
        token = strtok(NULL, " \t");
    }

    return argv;
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

void create_linux_program_child(char **argc, char *background){
    pid_t pid = fork();

    if(pid > 0){
    	if(*background == '\0'){
        	waitpid(pid, 0, 0);
    	}
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
   	char *argc[32] = {NULL}, background = '\0';
    int i, argv = input_arg_handler(cmdline, argc, &background);

    if(strcmp(argc[0], "exit") == 0){
    	for(i = 0; i < argv; i++) free(argc[i]);
        exit(0);
    }else if(strcmp(argc[0], "cd") == 0){
        change_dir(argc[1]);
    }else if(strcmp(argc[0], "child") == 0){
        create_child(argc[1], argc, argv);
    }else{
        create_linux_program_child(argc, &background);
    }


    for(i = 0; i < argv; i++) free(argc[i]);
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