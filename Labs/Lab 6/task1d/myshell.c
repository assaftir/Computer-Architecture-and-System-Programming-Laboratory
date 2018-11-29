#include <stdio.h>
#include "line_parser.h"
#include "job_control.h"
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>


#define READ_END 0
#define WRITE_END 1
#define NO_PIPE -1

void my_error(char *msg, int ret_value) {
	perror(msg);
	_exit(ret_value);
}


void set_input(cmd_line *cmd, int read_pipe){
	
	
    if(cmd->input_redirect && read_pipe != NO_PIPE) {
		printf("Ambiguous input redirect.\n");
	}
	
    if(cmd->input_redirect) {		
		close(STDIN_FILENO);
		fopen(cmd->input_redirect,"r");
	}
	else if(read_pipe != NO_PIPE){
		close(STDIN_FILENO);     /*a. Close the standard input.*/
		dup(read_pipe);           /*b. Duplicate the read-end of the pipe using dup, will get STDIN_FILENO*/
		close(read_pipe);         /*c. Close the file descriptor that was duplicated */
	}
}

void set_output(cmd_line *cmd, int write_pipe){
	
	
    if(cmd->output_redirect && write_pipe != NO_PIPE) {
		printf("Ambiguous output redirect.\n");
	}
	
    if(cmd->output_redirect) {		
		close(STDOUT_FILENO);
		fopen(cmd->output_redirect,"w");
	}
	else if(write_pipe != NO_PIPE){
		close(STDOUT_FILENO);     /*a. Close the standard output.*/
		dup(write_pipe);           /*b. Duplicate the write-end of the pipe using dup, will get STDOUT_FILENO*/
		close(write_pipe);         /*c. Close the file descriptor that was duplicated */
	}
}


void close_pipe_file(int fd) {
	if (fd != NO_PIPE) close(fd);
}

void set_gid(pid_t pid, job* job){
	if(job->pgid == 0){
		job->pgid = pid;
	}
	setpgid(pid, job->pgid);
}


void wait_for_job(job *job) { //Not in use!!! replaced by run_job_in_forground (task 1d)
	
	int sts=1;
	// When shell waits (if line->blocking is set), it should wait on the group instead of a single process
	// (using waitpid(-<group_id>, &status, options)). This should be done in a loop, and the loop should continue
	//as long as there are running children (using WNOHANG and checking for -1 return value).
	//printf("wait for: %d\n", job->pgid);
	while (waitpid(-(job->pgid), &sts, WNOHANG) != -1)
	job->status = DONE;
}

int execute_cmd(cmd_line *cmd, int *left_pipe, job* job) {
	
	if (!cmd) return FALSE;  /* Stop condition */
	
	pid_t chld;
	int right_pipe[]={NO_PIPE, NO_PIPE};
	
	if ((cmd->next) && (pipe(right_pipe) < 0)) { /* Create a pipe */
		my_error("pipe", EXIT_FAILURE);
	}
	
	if ((chld = fork()) == -1) {  /* Fork to a child process (child). */
		my_error("fork() error", EXIT_FAILURE);
	}
	
	/* In both the child and the parent (to avoid a racing condition): Set the process group id (pgid) of the new process to be the same as the child process id */
	
	
	
	if (chld == 0) {  /* On the child process do: */
	    
		/* n the child: Set the signal handlers back to default. */
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		
		// In both the child and the parent (to avoid a racing condition):
		// 1. Set the process group id of the child to be the same as the first child process id of the pipe. 
		// 2. If it is first child, save the group id in the job.
		if (job->pgid == 0) job->pgid = getpid();  //This condition is true only the first child 
		setpgid(getpid(), job->pgid);              // cann't use chld because chld = 0 for child
		
		//printf("Child: pid=%d, pgid=%d, job->pgid=%d\n", getpid(), getpgid(getpid()), job->pgid);
	
		//Execute child
		set_input(cmd, left_pipe[READ_END]);
		set_output(cmd, right_pipe[WRITE_END]);
		execvp(cmd->arguments[0], cmd->arguments);
		//If it arrives to the next command than execvp failed
		my_error("Execution has failed\n", EXIT_FAILURE);
	}
	else { /* The parent process */
	
		// In both the child and the parent (to avoid a racing condition):
		// 1. Set the process group id (pgid) of the child to be the same as the first child process id of the pipe. 
		// 2. If it is first child, save the group id in the job.
		if (job->pgid == 0) job->pgid = chld;  //This condition is true only the first child 
		setpgid(chld, job->pgid);   

		//printf("Shell: pid=%d, pgid=%d, job->pgid=%d\n", getpid(), getpgid(getpid()), job->pgid);

		close_pipe_file(left_pipe[READ_END]);
		close_pipe_file(right_pipe[WRITE_END]);
		
		return cmd->blocking || execute_cmd(cmd->next, right_pipe, job);	 //Only the last one might be TRUE
	}
	perror("Shouldnt be here!!");
	return 0;
}



void handle_signal(int signal) {
/*Task0a - prints the signal that the shell receives with a message saying it was ignored */

   // printf("%s%s\n", strsignal(signal), " has been ignored"); - No need for final task

}

int main(int argc, char **argv) {
	
	
	struct cmd_line *cmd;
	char my_cwd[PATH_MAX];
	char input[MAX_INPUT];
	int null_pipe[]={NO_PIPE,NO_PIPE};
	
	job* jobs = NULL;
	
	
	/* Task 1c - Initialization */
	/* Ignore the following signals: SIGTTIN, SIGTTOU, SIGTSTP, so they can reach the foreground child process rather than the shell. */
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	
	/* Handle the signals: SIGQUIT, SIGCHLD and not SIGINT so we can kill the shell with ^C if there's a bug somewhere */
	signal(SIGQUIT, handle_signal);
	signal(SIGCHLD, handle_signal);
	
	/* Set the process group of the shell to its process id */
	if (setpgid(getpid(), getpid()) < 0){
		perror("Unknown Error");
		exit(EXIT_FAILURE);
	}
	
	/* Save default terminal attributes to restore them back when a process running in the foreground ends.*/
	struct termios old_termios;
	tcgetattr(STDIN_FILENO, &old_termios);


	int isBlocking = FALSE;
	while (1){
	
		getcwd(my_cwd, PATH_MAX);
		printf("%s>> ", my_cwd);
		fgets(input, MAX_INPUT, stdin);
		cmd = parse_cmd_lines(input);  //Build the command line, returns null for enter
		if (!cmd) continue;  //Incase it is an empty command line, happens when user enter <enter>
		
		char *cmd_str = cmd->arguments[0];  /* just a temporary var  */

		if (strcmp(cmd_str, "quit")==0 || strcmp(cmd_str, "q")==0) {
			FREE(cmd);
			break;  /* Do not exit yet, need to free jobs */
		}
		else if (strcmp(cmd_str, "jobs")==0) {
			print_jobs(&jobs);
		}
		else if (strcmp(cmd_str, "fg")==0) {
			//printf("Shell2: pid=%d, pgid=%d\n", getpid(), getpgid(getpid()));
			run_job_in_foreground(&jobs, find_job_by_index(jobs, atoi(cmd->arguments[1])), CONTINUE, &old_termios, getpid());
		}

		else {
			job* job = add_job(&jobs, input);
			job->status = RUNNING;
			isBlocking = execute_cmd(cmd, null_pipe, job);
			if (isBlocking)
				run_job_in_foreground (&jobs, job, !CONTINUE, &old_termios, getpid());
		}	
		
		FREE(cmd);
		cmd=NULL;

	}
	free_job_list(&jobs);
	return 0;
}
	




