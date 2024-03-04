#include <sys/wait.h>
#include "sighandler.h"
#include "jobcontrol.h"

// debug
#include <stdio.h>

static void sigint_handler(int sig);
static void sigchld_handler(int sig);

static void sigsetinit();

sigset_t sigchld_set;
sigset_t sigint_set;
sigset_t empty_set;
sigjmp_buf env;

//When a process which has installed signal handlers forks, the child process
//inherits the signals.  All caught signals may be reset to their default
//action by a call to the execve(2) function; ignored signals remain ignored.
void siginit() {
    // remember to mask ctrl-c before you fork, cuz you don't want to have a forked shell to jump to main!
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGTTOU, SIG_IGN);
    sigsetinit();
}

// the forked child should set SIGINT, SIGTSTP, SIGCHLD to default
void siginit_child() {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}

static void sigsetinit() {
    sigemptyset(&empty_set);
    sigemptyset(&sigint_set);
    sigemptyset(&sigchld_set);
    sigaddset(&sigchld_set, SIGCHLD);
    sigaddset(&sigint_set, SIGINT);
}

static void sigint_handler(int sig) {
    siglongjmp(env, 1);
}

static void sigchld_handler(int sig) {
    int wait_status, wait_ret;
    // READ the MANNUAL carefully!!!! 0 means only wait for childrenof the same pgrp as you... should be -1
    while((wait_ret = waitpid(-1, &wait_status, WUNTRACED | WCONTINUED | WNOHANG)) != -1) {
        if(wait_ret == 0) {
            // no child wish to provide info
            break;
        }
        mod_job(wait_ret, wait_status);
    }
}
