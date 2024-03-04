#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>

#include "builtin.h"
#include "jobcontrol.h"
#include "error.h"

static void echo(int argc, char *argv[]);
static void cd(int argc, char* argv[]);
static void pwd();
//static void epxort(int argc, char* argv[]);
static void shell_exit();
static void jobs();
static void bg(int argc, char**);
static void fg(int argc, char**);
// static void _kill(char* argv[]);


static char* builtins[] = {"echo", "cd", "pwd", "exit", "jobs", "bg", "fg"};

int match_builtin(char* cmdname) {
    int i = 0;
    int num_builtin;

    num_builtin = sizeof(builtins) / sizeof(char*);
    for(i = 0; i < num_builtin; i++) {
        if(!strcmp(builtins[i], cmdname)) {
            return 1;
        }
    }
    return 0;
}

void do_builtin(cmd_t* cmd, int argc, char** argv) {
    char* cmdname;
    int old_stdin, old_stdout;
    int infile_fd, outfile_fd;

    old_stdin = 0;
    old_stdout = 0;

    if(cmd->infile != NULL) {
        infile_fd = open(cmd->infile, O_RDONLY);
        if(infile_fd == -1) {
            sys_error();
            goto done;
        }
        old_stdin = dup(STDIN_FILENO);
        dup2(infile_fd, STDIN_FILENO);
    }
    if(cmd->outfile != NULL) {
            outfile_fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(outfile_fd == -1) {
            goto done;
        }
        old_stdout = dup(STDOUT_FILENO);
        dup2(outfile_fd, STDOUT_FILENO);
    }

    cmdname = cmd->cmdname;
    if(!strcmp(cmdname, "echo")) {
        echo(argc, argv);
    } else if(!strcmp(cmdname, "cd")) {
        cd(argc, argv);
    } else if(!strcmp(cmdname, "pwd")) {
        pwd();
    } else if(!strcmp(cmdname, "exit")) {
        shell_exit();
    } else if(!strcmp(cmdname, "jobs")) {
        jobs();
    } else if(!strcmp(cmdname, "bg")) {
        bg(argc, argv);
    } else if(!strcmp(cmdname, "fg")) {
        fg(argc, argv);
    }

done:
    if(outfile_fd > 0) {
        dup2(old_stdout, STDOUT_FILENO);
    }
    if(infile_fd > 0) {
        dup2(old_stdin, STDIN_FILENO);
    }

    return;
}

static void echo(int argc, char *argv[]) {
    int i = 0;
    for(i = 1; i < argc; i++) {
        printf("%s%c", argv[i], i < argc - 1 ? ' ' : '\n');
    }
}

// Interesting... if I declare char* I'll get SIGBUS from cd but
// not pwd... to investigate
extern char cwd[MAXPATHLEN];
static void cd(int argc, char *argv[]) {
    if(argc > 1) {
        if(chdir(argv[1]) == -1) {
            perror("cd");
            return;
        }
    } else {
        if(chdir(getenv("HOME")) == -1) {
            perror("cd");
            return;
        }
    }
    getcwd(cwd, MAXPATHLEN);
}

static void pwd() {
    printf("%s\n", cwd);
}

static void shell_exit() {
    exit(0);
}

static void bg(int argc, char* argv[]) {
    pid_t pgrp;
    if(argc < 2) {
        error("bg: usage: bg <process group id>");
    }
    pgrp = (pid_t)strtol(argv[1], NULL, 10);
    if(pgrp != 0) {
        bg_job(pgrp);
    }
}

static void fg(int argc, char* argv[]) {
    pid_t pgrp;

    if(argc < 2) {
        error("fg: usage: fg <process group id>");
    }
    pgrp = (pid_t)strtol(argv[1], NULL, 10);
    if(pgrp != 0) {
        fg_job(pgrp);
    }
}

static void jobs() {
    list_jobs();
}

// TODO finish this after local variables are supported
// static void export(int argc, char *argv[]) {
//   for ( int i = 1; ; i++ ) {
//     if ( putenv(argv[i]) != 0 ) {
//       perror("export");
//     }
//   }
// }
