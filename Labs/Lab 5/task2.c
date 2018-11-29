
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define READ_END 0
#define WRITE_END 1

void my_error(char *msg, int ret_value) {
	perror(msg);
	_exit(ret_value);
}


int main(int argc, char *argv[]) {

	int _pipe[2];
	pid_t chld1, chld2;
	
	/*1. Create a pipe */
	if (pipe(_pipe) == -1) {
		my_error("pipe", EXIT_FAILURE);
	}
	
	/*2. Fork to a child process (child1). */
	chld1 = fork();
	if (chld1 == -1) {
		my_error("fork() error", EXIT_FAILURE);
	}
	
	/*3. On the child1 process do: */
	if (chld1 == 0) {					 
		close(STDOUT_FILENO);   /*a. Close the standard output.*/
		dup(_pipe[WRITE_END]);          /*b. Duplicate the write-end of the pipe using dup, will get STDOUT_FILENO*/
		close(_pipe[WRITE_END]);        /*c. Close the file descriptor that was duplicated */
		char* exec_args[3] = {"ls", "-l", NULL};   /*d. Execute "ls -l" */
		execvp(exec_args[0], exec_args);
		my_error("Unknown error\n", EXIT_FAILURE);
	}
	else {	/* On the parent process:  */
		close(_pipe[WRITE_END]);    /*4. Close the write end of the pipe. */
		chld2 = fork();			/*5. Fork again to a child process (child2). */
		if (chld2 == -1) {
			my_error("fork() error\n", EXIT_FAILURE);
		}
		if (chld2 == 0) {	/* On the child2 process do: */
			close(STDIN_FILENO);   /*a. Close the standard input.*/
			dup(_pipe[READ_END]);         /*b. Duplicate the read-end of the pipe using dup (see man)*/
			close(_pipe[READ_END]);       /*c. Close the file descriptor that was duplicated */
			char* exec_args[4] = {"tail", "-n", "2", NULL};   /*d. Execute "tail -n 2". */
			execvp(exec_args[0], exec_args);
			my_error("Unknown error\n", EXIT_FAILURE);
		}
		else {		/* On the parent process: */
			close(_pipe[READ_END]);          /*7. Close the read end of the pipe.  */
			waitpid(chld2, NULL, 0);  /* Wait for the child 2 processes to terminate */
		}
	}
}

