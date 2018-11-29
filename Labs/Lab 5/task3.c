#include <stdio.h>
#include "line_parser.h"
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>


#define READ_END 0
#define WRITE_END 1

void my_error(char *msg, int ret_value) {
	perror(msg);
	_exit(ret_value);
}


void io_redirect(int stdfile, const char *redirect_file, char *flag) {
	if(redirect_file) {
		//By closing the standard input stream and opening a new file, which in turn is
		//automatically allocated with the lowest available file-descriptor index, it effectively overrides the standard input.
		close(stdfile);
		fopen(redirect_file,flag);
	}
}

void execute_no_pipe(cmd_line *cmd) {
		
	io_redirect(STDOUT_FILENO, cmd->output_redirect, "w");
	io_redirect(STDIN_FILENO, cmd->input_redirect, "r");
	
	execvp(cmd->arguments[0], cmd->arguments);
	//If it arrives to the next command than execvp failed
	my_error("Execution has failed\n", EXIT_FAILURE);		
}

void run_no_pipe(cmd_line *cmd) {

	pid_t pid;
 
	if ((pid = fork()) < 0) {
		my_error("Fork failed", EXIT_FAILURE);
	}

	if (pid == 0){   //If a child process
		execute_no_pipe(cmd);
		free_cmd_lines(cmd);
	}
	else if (cmd->blocking) {
		waitpid(pid, NULL, WUNTRACED);  //Wait until the child terminates
	}
}

void execute_write_end(cmd_line *cmd, int write_pipe){

    if(cmd->output_redirect) {
		printf("Ambiguous output redirect.\n");
	}
	
	io_redirect(STDIN_FILENO, cmd->input_redirect, "r");

	close(STDOUT_FILENO);     /*a. Close the standard output.*/
	dup(write_pipe);          /*b. Duplicate the write-end of the pipe using dup, will get STDOUT_FILENO*/
	close(write_pipe);        /*c. Close the file descriptor that was duplicated */
	execvp(cmd->arguments[0], cmd->arguments);
	//If it arrives to the next command than execvp failed
	my_error("Execution has failed\n", EXIT_FAILURE);
}

void execute_read_end(cmd_line *cmd, int read_pipe){

    if(cmd->input_redirect) {
		printf("Ambiguous input redirect.\n");
	}
	
	io_redirect(STDOUT_FILENO, cmd->output_redirect, "w");

	close(STDIN_FILENO);     /*a. Close the standard input.*/
	dup(read_pipe);          /*b. Duplicate the read-end of the pipe using dup, will get STDIN_FILENO*/
	close(read_pipe);        /*c. Close the file descriptor that was duplicated */
	execvp(cmd->arguments[0], cmd->arguments);
	//If it arrives to the next command than execvp failed
	my_error("Execution has failed\n", EXIT_FAILURE);
}

void run_with_pipe(cmd_line *cmd) {	

	int _pipe[2];
	pid_t chld1, chld2;

	if (pipe(_pipe) < 0) { /* Create a pipe */
		my_error("pipe", EXIT_FAILURE);
	}
	
	if ((chld1 = fork()) == -1) {  /* Fork to a child process (child1). */
		my_error("fork() error", EXIT_FAILURE);
	}
	
	if (chld1 == 0) {  /* On the child1 process do: */
		execute_write_end(cmd, _pipe[WRITE_END]);
		exit(0);  /* So the child doesn't end up in the main program */
	}
	else {	/* On the parent process:  */
		close(_pipe[WRITE_END]);    /* Close the write end of the pipe. */
		if ((chld2 = fork()) == -1) {  /* Fork again to a child process (child2). */
			my_error("fork() error\n", EXIT_FAILURE);
		}
		if (chld2 == 0) {	/* On the child2 process do: */
			execute_read_end(cmd->next, _pipe[READ_END]);
			free_cmd_lines(cmd);
			exit(0);  /* So the child doesn't end up in the main program */
		}
		else {		/* On the parent process: */
			close(_pipe[READ_END]);          /*7. Close the read end of the pipe.  */
			if (cmd->next->blocking)
				waitpid(chld2, NULL, 0);  /* Wait for the child 2 processes to terminate */
		}
	}
}

int main(int argc, char **argv) {
	
	
	struct cmd_line * my_line;
	char my_cwd[PATH_MAX];
	char input[MAX_INPUT];
 
	while (1){
	
		getcwd(my_cwd, PATH_MAX);
		printf("%s>> ", my_cwd);
		fgets(input, MAX_INPUT, stdin);
		if (strcmp(input, "quit\n")==0 || strcmp(input, "q\n")==0) {
			exit(0);
		}

		my_line = parse_cmd_lines(input);  //Build the command line, returns null for enter
		if (!my_line) continue;  //Incase it is an empty command line, happens when user enter <enter>

		if (!my_line->next) {
			run_no_pipe(my_line);
		}
		else {
			run_with_pipe(my_line);
		}
	}
	return 0;
}
	




