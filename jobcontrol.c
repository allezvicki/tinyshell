#include <malloc.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "parsetypes.h"
#include "jobcontrol.h"
#include "error.h"
#include "sighandler.h"

//debug
#include <stdio.h>

// from interpret.c
extern int fg_goingon;

// FIXME this is buggy. need to maintain a proc list instead
// why? cannot handle when a proc is stopped before it is killed
// when number of terminated = proc num, delete job
// when term + stopped = all, job is stopped
// if there is any running proc, job is running
// TODO: support job id
// int job_id;
struct job {
    int is_fg;
    char* cmd;
    pid_t pgrp;
    pid_t last_pid;
    int num_running;
    int num_terminated;
    int num_proc;
    struct job* prev;
    struct job* next;    
};

// dumb-head
static struct job dumb_head;
static struct job* tail_ptr = &dumb_head;

void add_job(char* cmd, pid_t pgrp, int numproc, int is_fg, pid_t last_pid) {
    struct job* job_ptr;
    char* cmd_ptr;

    job_ptr = (struct job*) malloc(sizeof(struct job)); 
    if(job_ptr == NULL) {
        sys_fatal_error();
    }
    cmd_ptr = (char*) malloc(sizeof(char) * (strlen(cmd) + 1));
    strncpy(cmd_ptr, cmd, strlen(cmd));
    cmd_ptr[strlen(cmd)] = '\0';
    if(cmd_ptr == NULL) {
        sys_fatal_error();
    }

    job_ptr->is_fg = is_fg;
    job_ptr->last_pid = last_pid;
    job_ptr->cmd = cmd_ptr;
    job_ptr->pgrp = pgrp;
    job_ptr->num_running = numproc;
    job_ptr->num_proc = numproc;
    job_ptr->num_terminated = 0;
    job_ptr->prev = tail_ptr;
    job_ptr->next = NULL;
    tail_ptr->next = job_ptr;
    tail_ptr = job_ptr;
}

void delete_job(struct job* job_ptr) {
    struct job* next_job_ptr;
    struct job* prev_job_ptr;
    
    next_job_ptr = job_ptr->next;
    prev_job_ptr = job_ptr->prev;
    if(next_job_ptr != NULL){
        next_job_ptr->prev = prev_job_ptr;
    }
    if(prev_job_ptr != NULL) {
        prev_job_ptr->next = next_job_ptr;
    }
    if(job_ptr->is_fg) {
        // job done
        fg_goingon = 0;
    }
    if(job_ptr == tail_ptr) {
        tail_ptr = prev_job_ptr;
    }
    free(job_ptr->cmd);
    free(job_ptr);
}

void mod_job(pid_t pid, int status) {
    // FIXME what if the child changed is group?
    // we need to maintain a proc list for every group...
    // settle for this implementation now.
    int sig;
    struct job* job_ptr;

    // debug
    job_ptr = dumb_head.next;
    // oh but if the child is dead how are you going to getpgid????
    // pgrp = getpgid(pid);    
    // SOLUTION: use last_pid to determine the real pgrp of the child
    while(job_ptr != NULL) {
        if(pid >= job_ptr->pgrp && pid <= job_ptr->last_pid) {
            if(WIFEXITED(status) || WIFSIGNALED(status)) {
                job_ptr->num_terminated++;
                // FIXME yes this is a bug, what if the proc was stopped before it was killed?
                // have to maintain a proc list to really know if a job is stopped or not
                // job_ptr->num_running--;
                if(job_ptr->num_terminated == job_ptr->num_proc) {
                    delete_job(job_ptr);
                }
            } else if(WIFSTOPPED(status)) {
                job_ptr->num_running--;
                if(job_ptr->num_running == 0) {
                    // job stopped
                    if(job_ptr->is_fg) {
                        job_ptr->is_fg = 0;
                        fg_goingon = 0;
                    }
                }
                // STOPSIG
                // sig = WSTOPSIG(status);
                // if(job_ptr->is_fg && (sig == SIGTTOU || sig == SIGTTIN)) {
                //     kill(-job_ptr->pgrp, SIGCONT);
                // }
            } else if(WIFCONTINUED(status)) {
                job_ptr->num_running++;
            }
            return;
        }
        job_ptr = job_ptr->next;
    }
}

char* jobstatus(struct job* job) {
    if(job->num_running > 0) {
        return "Running";
    } else {
        return "Stopped";
    }
}

void list_jobs() {
    struct job* job_ptr;
    
    job_ptr = dumb_head.next;
    while(job_ptr != NULL) {
        printf("[%d] %s     %s", job_ptr->pgrp, jobstatus(job_ptr), job_ptr->cmd);
        job_ptr = job_ptr->next;
    }
}

void fg_job(pid_t pgrp) {
    struct job* job_ptr;
    
    job_ptr = dumb_head.next;
    // job_ptr is save because SIGCHLD is blocked right now
    while(job_ptr != NULL) {
        if(job_ptr->pgrp == pgrp) {
            // FIXME use ttyfd to make "jobs < foo" ok...
            if(tcsetpgrp(STDIN_FILENO, pgrp) != -1) {
                job_ptr->is_fg = 1;
                fg_goingon = 1;
                kill(-pgrp, SIGCONT);
                while(fg_goingon) {
                    sigsuspend(&empty_set);
                }
                tcsetpgrp(STDIN_FILENO, getpgrp());
            }
            return;
        }
        job_ptr = job_ptr->next;
    }
}

void bg_job(pid_t pgrp) {
    struct job* job_ptr;
    
    job_ptr = dumb_head.next;
    while(job_ptr != NULL) {
        if(job_ptr->pgrp == pgrp) {
            kill(-pgrp, SIGCONT);
            printf("Running background job [%d] %s\n", pgrp, job_ptr->cmd);
            return;
        }
        job_ptr = job_ptr->next;
    }
    error("bg: no such job");
}
