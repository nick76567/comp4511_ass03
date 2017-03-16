#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

// remember to close the unused pipefd



void test(char **cmd, int *stdin_fd, const int all_cmdline_size){
	int pipefd[2];
	pipe(pipefd);
	
	char c;
	int i;
	*stdin_fd = dup(0);
	pid_t pid;

	for(i = 0; i < all_cmdline_size; i++){
		pipe(pipefd);



		pid = fork();



		if(pid == 0){
			printf("%d %d\n", fcntl(0, F_GETFD), i);
			while(read(0, &c, 1) != 0){
				if(c == ']') break;
					printf("%c", c);
				}

			if(fcntl(1, F_GETFD) != -1) close(1);
			dup2(pipefd[1], 1);
			close(pipefd[0]);

			execlp(cmd[i], cmd[i], (char *)NULL);
		}else{


				wait(0);				
				if(fcntl(0, F_GETFD) != -1) close(0);				 
				dup2(pipefd[0], 0);
				close(pipefd[1]);
				
				close(pipefd[0]);

				printf("yes\n");
				
	

			
		}
		
	}



/*
	while(write(0, &c, 1) != 0){
		printf("%c", c);
	}
*/
}

int main(){
	char *arg[3] = {"ps", "sort", "less"};
	int stdin_fd;
	test(arg, &stdin_fd, 3);

/*	
	int pipefd[2];
	char buffer;

	if(pipe(pipefd) == -1){
		perror("Error occurs in creating pipe\n");
		exit(-1);
	}
	pid_t pid = fork();

	if(pid == 0){
		if(close(1) == -1){
			perror("Error occurs in closing file descriptor\n");
			exit(-1);
		}

		if(dup2(pipefd[1], 1) == -1){
			perror("Error occurs in duplicating file descriptor\n");
			exit(-1);
		}

		if(close(pipefd[0]) == -1){
			perror("Error occurs in closing file descriptor\n");
			exit(-1);
		}

		execlp("cat", "cat", "hello_world.txt", (char *)NULL);

	}else if(pid > 0){
		int temp_stdin_fd;
		char c;

		if((temp_stdin_fd = dup(0)) == -1){
			perror("Error occurs in duplicating file descriptor\n");
			exit(-1);
		}

		if(close(0) == -1){
			perror("Error occurs in closing file descriptor\n");
			exit(-1);
		}

		if(dup2(pipefd[0], 0) == -1){
			perror("Error occurs in duplicating file descriptor\n");
			exit(-1);
		}

		if(close(pipefd[1]) == -1){
			perror("Error occurs in closing file descriptor\n");
			exit(-1);
		}

		while(read(0, &c, 1) != 0){
			printf("%c", c);
		}

		if(close(0) == -1){
			perror("Error occurs in closing file descriptor\n");
			exit(-1);
		}

		if(dup2(temp_stdin_fd, 0) == -1){
			perror("Error occurs in duplicating file descriptor\n");
			exit(-1);
		}

		if(close(temp_stdin_fd) == -1){
			perror("Error occurs in closing file descriptor\n");
			exit(-1);
		}
		
		scanf("%c", &c);
		printf("\n%c\n", c);
		
	}else{
		perror("Error occurs in input redirection fork\n");
		exit(-1);
	}
*/	

/*
	char c;
	int temp_stdin = dup2(0, 3);
	close(0);
	//int fd = open("hello_world.txt", O_RDONLY);

	while(read(STDIN_FILENO, &c, 1) != 0){
		printf("%c", c);
	}
*/

/*
	int pipefd[2];
	pipe(pipefd);
	char c;

	pid_t pid = fork();

	if(pid == 0){
		//int fd = open("hello_world.txt", O_RDONLY);

		close(1);
		dup2(pipefd[1], 1);
		close(pipefd[0]);
		execlp("cat", "cat", "hello_world.txt", (char *)NULL);
		exit(0);

	}else if(pid > 0){
		close(0);
		dup2(pipefd[0], 0);
		close(pipefd[1]);
		wait(0);
		//printf("%s\n", stdin);
		while(1){
			if(read(0, &c, 1) <= 0) break;
			printf("%c", c);
			
		}
		
	}
*/
	return 0;

/*
	//fscanf(stdin, "testtest\n");
	int cstdin = dup(0);
	close(0);
	printf("%d\n", cstdin);
	dup(3);
	char c;

	do{
		c = fgetc(stdin);
		if(feof(stdin)){
			break;
		}
		printf("%c", c);

	}while(1);
*/

	//fprintf(stdout, "test\n");
	//close(1);

/*	
	int fd;
	int pfds[2];
	pipe(pfds);
	char buf, c;
	
	pid_t pid = fork();

	if(pid == 0){
		//fd = open("hello_world.txt", O_RDONLY);

		if(close(1) == -1){
			printf("Error in close stdout file descriptor\n");
			exit(-1);
		}

		if(dup2(pfds[1], 1) == -1){
			printf("Error in duplicating file descriptor\n");
			exit(-1);
		}

		if(close(pfds[0]) == -1){
			printf("Error in closing pipe write side\n");
			exit(-1);
		}

		FILE *fin = fopen("hello_world.txt", "r");
		do{
			buf = fgetc(fin);
			if(feof(fin)) break;
			printf("%c", buf);
		}while(1);

		fclose(fin);
	}else{
		if(close(0) == -1){
			printf("Error in close stdin file descriptor\n");
			exit(-1);
		}

		if(dup2(pfds[0], 0) == -1){
			printf("Error in duplicating file descriptor\n");
			exit(-1);
		}

		if(close(pfds[1]) == -1){
			printf("Error in closing pipe write side\n");
			exit(-1);
		}

		wait(0);
		while(read(pfds[0], &c, 1) > 0){
			printf("%c", c);
		}

	}
*/	
}