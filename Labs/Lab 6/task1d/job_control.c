#include "job_control.h"



/**
* Receive a pointer to a job list and a new command to add to the job list and adds it to it.
* Create a new job list if none exists.
**/
job* add_job(job** job_list, char* cmd){
	job* job_to_add = initialize_job(cmd);
	
	if (*job_list == NULL){
		*job_list = job_to_add;
		job_to_add -> idx = 1;
	}	
	else{
		int counter = 2;
		job* list = *job_list;
		while (list -> next !=NULL){
			printf("adding %d\n", list->idx);
			list = list -> next;
			counter++;
		}
		job_to_add ->idx = counter;
		list -> next = job_to_add;
	}
	return job_to_add;
}


/**
* Receive a pointer to a job list and a pointer to a job and removes the job from the job list 
* freeing its memory.
**/
void remove_job(job** job_list, job* tmp){
	
	if (*job_list == NULL)
		return;
	job* tmp_list = *job_list;
	if (tmp_list == tmp){
		*job_list = tmp_list -> next;
		free_job(tmp);
		return;
	}
	while (tmp_list->next != tmp){
		tmp_list = tmp_list -> next;
	}
	tmp_list -> next = tmp -> next;
	free_job(tmp);
	
}

/**
*   Receive a job list, and print it in the following format:<code>[idx] \t status \t\t cmd</code>, where:
    cmd: the full command as typed by the user.
    status: Running, Suspended, Done (for jobs that have completed but are not yet removed from the list).
  
**/
void print_jobs(job** job_list){

	job* tmp = *job_list;
	update_job_list(job_list, FALSE);  /* First update the list to be up-to-date then print it */
	while (tmp != NULL){
		
		printf("[%d]\t %s \t\t %s", tmp->idx, status_to_str(tmp->status),tmp -> cmd); 
		if (tmp -> cmd[strlen(tmp -> cmd)-1]  != '\n')
			printf("\n");

		job* job_to_remove = tmp;
		
		tmp = tmp -> next;
		if (job_to_remove->status == DONE) {
			remove_job(job_list, job_to_remove);
		}		
	}
}


/**

* Receive a pointer to a list of jobs, and delete all of its nodes and the memory allocated for each of them.
*/
void free_job_list(job** job_list){
	while(*job_list != NULL){
		job* tmp = *job_list;
		*job_list = (*job_list) -> next;
		free_job(tmp);
	}
	
}


/**
* receives a status and prints the string it represents.
**/
char* status_to_str(int status){
	static char* strs[] = { "Done", "Suspended", "Running" };
	return strs[status + 1];
}


/************************************* MY CODE *****************************/

/**
* receives a pointer to a job, and frees it along with all memory allocated for its fields.
**/
void free_job(job* job_to_remove){
	if(job_to_remove != NULL){
		FREE(job_to_remove->cmd);
		FREE(job_to_remove);
	}

}



/**
* Receive a command (string) and return a job pointer. 
* The function needs to allocate all required memory for: job, cmd, tmodes
* to copy cmd, and to initialize the rest of the fields to NULL: next, pigd, status 
**/

job* initialize_job(char* cmd){
	
	job* j = (job *)malloc(sizeof(job));
	j->cmd = (char*)malloc(strlen(cmd) + 1);
	strcpy(j->cmd, cmd);
	j->next = NULL;
	j->status = 0;
	j->pgid = 0;
	j->idx = 0;
	j->tmodes = NULL;
	return j;
}


/**
* Receive a job list and and index and return a pointer to a job with the given index, according to the idx field.
* Print an error message if no job with such an index exists.
**/
job* find_job_by_index(job* job_list, int idx){

	for (;job_list; job_list = job_list->next) {
		if (job_list->idx == idx) 
			return job_list;
	}
	
	printf("Job %d not found\n", idx);
	return NULL;

}

/**
* Update the status of one job running in the background to DONE
* Receive a pointer to a job list, and a boolean to decide whether to remove it
* from the job list if job is done. 
* returns the job or NULL if the job was removed
**/
job *update_one_job(job **job_list, job *job, int remove_done_job) {
	
	int sts = 0;
	int pid;
	
	if (job->status != DONE) {  //DONE = Already been there!!
		pid = waitpid(-(job->pgid), &sts, WNOHANG);
		if (pid < 0) {
			job->status = DONE;
		}
		else if (pid > 0 && (WIFEXITED(sts) || WIFSIGNALED(sts)))  {
			job->status = DONE;
		}
		else if(WIFSTOPPED(sts)){    //Shouldnt be here!!     
			perror("********** job suspended!!\n");
			job->status = SUSPENDED;
		}
	}
		
	if(job->status == DONE && remove_done_job){
		printf("[%d]\t %s \t\t %s\n", job->idx, status_to_str(job->status), job->cmd);		
		remove_job(job_list, job);
		return NULL;
	}
	return job;
}

/**
* Update the status of jobs running in the background to DONE
* Receive a pointer to a job list, and a boolean to decide whether to remove done
* jobs from the job list or not. 
**/
void update_job_list(job **job_list, int remove_done_jobs) {
	
	if (!job_list || !*job_list)
		return;
		
	job *curr, *next;
	for(curr = *job_list; curr; curr=next) { 
		next=curr->next; //curr might be removed within the loop that why it needs to hold of "next" upfront
		update_one_job(job_list, curr, remove_done_jobs);			
	}

}

/** 
* Put job job in the foreground.  If cont is nonzero, restore the saved terminal modes and send the process group a
* SIGCONT signal to wake it up before we block.  Run update_job_list to print DONE jobs.
**/

void run_job_in_foreground (job** job_list, job *job, int cont, struct termios* shell_tmodes, pid_t shell_pgid){
	
	if (!job_list || !job)
		return;
	
 	int status;
	
	job = update_one_job(job_list, job, TRUE);  //Check if the job is done
	if (!job) // job == NULL if is DONE
		return;
		
	job->tmodes = shell_tmodes;
	tcsetpgrp(STDIN_FILENO, job->pgid); //put it in the foreground
	
	if (cont && job->status == SUSPENDED) {
		tcsetattr(STDIN_FILENO, TCSADRAIN, job->tmodes); //set the attributes of the terminal to the one of the job
		if (kill(job->pgid, SIGCONT) < 0)  //send SIGCONT signal to the process group of the job
			perror("kill error");
	}
	
	
	waitpid(job->pgid, &status, WUNTRACED); //Wait for the job to change status
	if (WIFSTOPPED(status))
		job->status = SUSPENDED; //Change the status of the job to SUSPENDED if received SIGTSTP (ctrl-z) 
	else
		job->status = DONE; //Any other case change the status of the job to DONE 
							//include if received SIGINT (ctrl-c)
	
	tcsetpgrp(STDIN_FILENO, shell_pgid);  //Put the shell back in the foreground
	tcgetattr(STDIN_FILENO, job->tmodes); //Save the terminal attributes in the job tmodes.
	tcsetattr(STDIN_FILENO, TCSADRAIN, shell_tmodes); //Restore the shell’s terminal attributes to 
													  //prevent leaving the shell in an unstable mode
	update_job_list(job_list, 0); // Check for status update of jobs

}

/** 
* Put a job in the background.  If the cont argument is nonzero, send
* the process group a SIGCONT signal to wake it up.  
**/

void run_job_in_background (job *job, int cont){	
 	if (cont) {
		job->status = RUNNING;
		if (kill(job->pgid, SIGCONT) < 0)
			perror("kill error");
	}
}