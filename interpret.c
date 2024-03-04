#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#include "jobcontrol.h"
#include "parsetypes.h"
#include "error.h"
#include "sighandler.h"
#include "builtin.h"

//debug
#include <stdio.h>

static char** make_argv(cmd_t* cmd);
int fg_goingon;

// About pipes:
// before forking, shell should have prev[0] and pipe[1] open
// prev[1] is useless now, and pipe[0] should be preserved

void interpret(root_t* root, char* input) {
    int isbg, is_first_cmd, has_next_cmd;
    int prev_pipe_fd[2];
    int pipe_fd[2];
    int infile_fd, outfile_fd;
    int cmdnum;
    cmd_t* cmd;
    list_item *list_ptr, *next_list_ptr;
    char** argv;
    pid_t fork_ret, pipeline_pgid;
    pid_t self_pgrp;

    cmdnum = 0;
    self_pgrp = getpgrp();    
    is_first_cmd = 1;
    isbg = root->isbg;
    list_ptr = root->cmds.cmdlist_head;

    if(root->cmds.cmdlist_head == NULL) {
        return;
    }

    // Block SIGINT so forked child won't receive it and behave weird
    // sigprocmask(SIG_BLOCK, &sigint_set, NULL);
    // Block sigchld to receive child info after all children are forked
    sigprocmask(SIG_BLOCK, &sigchld_set, NULL);

    if(!isbg) {
        fg_goingon = 1;
    }


    while(list_ptr != NULL) {
        cmdnum++;
        prev_pipe_fd[0] = pipe_fd[0];
        prev_pipe_fd[1] = pipe_fd[1];
        cmd = (cmd_t*) list_ptr->ptr;
        next_list_ptr = (list_item*) list_ptr->next;
        // argv is malloced
        argv = make_argv(cmd);

        // built-ins are to be executed without spawning new process
        // if built-in is in a pipeline, there's /usr/bin/cd and alike ... anyway

        // has next cmd?
        if(next_list_ptr != NULL) {
            has_next_cmd = 1;
            if(pipe(pipe_fd)) {
                sys_fatal_error();
            }
        } else {
            has_next_cmd = 0;
        }

        // if this is the only cmd, check if built in
        // do_builtin returns true if it is builtin
        if(!has_next_cmd && is_first_cmd && match_builtin(cmd->cmdname)) {
            do_builtin(cmd, cmd->argnum + 1, argv); 
            free(argv);
            // unblock sigchld
            sigprocmask(SIG_UNBLOCK, &sigchld_set, NULL);
            return;
        }

        fork_ret = fork();
        switch(fork_ret) {
            case -1:
                sys_fatal_error();
            case 0:
                // child
                // file
                // Install default handler for SIGINT then unblock it
                // also install other default handlers
                // signal(SIGINT, SIG_DFL);
                siginit_child();
                sigprocmask(SIG_UNBLOCK, &sigint_set, NULL);
                sigprocmask(SIG_UNBLOCK, &sigchld_set, NULL);

                if(!is_first_cmd) {
                    setpgid(0, pipeline_pgid);
                } else {
                    setpgid(0, 0);
                }

                if(cmd->infile != NULL) {
                    infile_fd = open(cmd->infile, O_RDONLY);
                    if(infile_fd == -1) {
                        goto child_error;
                    }
                    dup2(infile_fd, STDIN_FILENO);
                    //no need to do this...
                    //close(infile_fd);
                }
                if(cmd->outfile != NULL) {
                    outfile_fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if(outfile_fd == -1) {
                        goto child_error;
                    }
                    dup2(outfile_fd, STDOUT_FILENO);
                    //close(outfile_fd);
                }

                // pipe
                if(!is_first_cmd) { // rediret stdin
                    dup2(prev_pipe_fd[0], STDIN_FILENO);
                }
                if(has_next_cmd) {
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[0]);
                }
                
                execvp(cmd->cmdname, argv);
child_error:
                sys_error();
                // use _exit() so the duplicated stdio buffer won't be flushed
                _exit(0);
            default:
                // shell

                if(is_first_cmd) {
                    pipeline_pgid = fork_ret;    
                }
                // set child pgid
                setpgid(fork_ret, pipeline_pgid);
                if(!has_next_cmd) {
                    add_job(input, pipeline_pgid, cmdnum, !isbg, fork_ret);
                }
                if(has_next_cmd) {
                    // close the write end of the current pipe
                    close(pipe_fd[1]);
                }
                if(!is_first_cmd) {
                    // close the read end of the prev pipe
                    close(prev_pipe_fd[0]);
                }
                break;
        }

        list_ptr = next_list_ptr;
        is_first_cmd = 0;
        free(argv);
    }

    // unblock sigchld so we can modify job status
    sigprocmask(SIG_UNBLOCK, &sigchld_set, NULL);
    if(!isbg) {
        if(tcsetpgrp(STDIN_FILENO, pipeline_pgid) == -1) {
            return;
        }
        kill(-pipeline_pgid, SIGCONT);
        while(fg_goingon) {
            sigsuspend(&empty_set);
        }
        // why ktrace did not catch this SIGTTOU?
        // anyway, have to ignore sigttou...
        tcsetpgrp(STDIN_FILENO, self_pgrp);
    }
    // Unblock sigint
    // sigprocmask(SIG_UNBLOCK, &sigint_set, NULL);
}

static char** make_argv(cmd_t* cmd) {
    list_item* list_ptr;
    char** argv;
    int i;

    list_ptr = cmd->arglist_head;
    argv = (char**)malloc(sizeof(char*) * (cmd->argnum + 2));
    if(argv == NULL) {
        sys_fatal_error();
    }
    argv[0] = cmd->cmdname;
    i = 1;
    while(list_ptr != NULL) {
        argv[i++] = (char*) list_ptr->ptr;
        list_ptr = list_ptr->next;
    }
    argv[i] = NULL;
    return argv;
}
