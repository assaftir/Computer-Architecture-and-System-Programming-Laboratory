#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "line_parser.h"
#include <sys/wait.h>
#include <unistd.h>



void execute(cmd_line *cmd){

    if(cmd->output_redirect) {
		//By closing the standard output stream and opening a new file, which in turn is
		//automatically allocated with the lowest available file-descriptor index, it effectively overrides stdout.
		fclose(stdout);
		fopen(cmd->output_redirect,"w");
	}

    if(cmd->input_redirect) {
		//By closing the standard input stream and opening a new file, which in turn is
		//automatically allocated with the lowest available file-descriptor index, it effectively overrides stdin.
		fclose(stdin);		
		fopen(cmd->input_redirect,"r");
	}
		
	execvp(cmd->arguments[0], cmd->arguments);
	//If it arrives to the next command than execvp failed
	perror("Execution has failed\n");
	exit(1);
}


int main(int argc, char **argv) {

	struct cmd_line * my_line;
	char my_cwd[PATH_MAX];
	char input[MAX_INPUT];
	int pid;
 
	while (1){

		getcwd(my_cwd, PATH_MAX);
		printf("%s>> ", my_cwd);
		fgets(input, MAX_INPUT, stdin);
		if (strcmp(input, "quit\n")==0 || strcmp(input, "q\n")==0) {
			exit(0);
		}


		my_line = parse_cmd_lines(input);  //Build the command line, returns null for enter
		if (!my_line) continue;  //Incase it is an empty command line, happens when user enter <enter>

		if ((pid = fork()) < 0) {
			_exit(-1); //The function _exit() terminates the calling process "immediately" and the process’s parent is sent a SIGCHLD signal.
		}

		if (pid == 0){   //If a child process
			execute(my_line);
			free_cmd_lines(my_line);
		}
		else if (my_line->blocking) {
			waitpid(pid, NULL, WUNTRACED);  //Wait until the child terminates
		}


	}	

	return 0;

}