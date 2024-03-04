#include <unistd.h>
struct job;

void add_job(char* cmd, pid_t pgrp, int numproc, int is_fg, pid_t last_pid);
void delete_job(struct job* job_ptr);
void mod_job(pid_t pid, int status);
void fg_job(pid_t pgrp);
void bg_job(pid_t pgrp);
void list_jobs();
