#include <stdio.h>
#include <setjmp.h>
// freebsd specific?
#include <sys/param.h>
#include <unistd.h>
#include <signal.h>

#include "error.h"
#include "parsetypes.h"
#include "interpret.h"
#include "sighandler.h"
#include "lex.yy.h"


#define MAXINPUT 1024

char cwd[MAXPATHLEN];
static char hostname[MAXHOSTNAMELEN];
static char input[MAXINPUT];
static char* username;
//char username[MAXLOGNAME];
static void print_prompt();
static void init_prompt();

// from shell.y
extern void yyparse();
extern root_t cmds_root;
extern int parse_error;

int main() {
    root_t root;
    YY_BUFFER_STATE yybuf;
    // debug
    // setvbuf(stdout, NULL, _IONBF, 0);
    init_prompt();
    siginit();
    // block sigint
    sigprocmask(SIG_BLOCK, &sigint_set, NULL);
    while(1) {
        if(sigsetjmp(env, 1)) {
            printf("\n");
        }
        print_prompt();
        fflush(stdin);
        // unblock sigint
        sigprocmask(SIG_UNBLOCK, &sigint_set, NULL);
        if(fgets(input, MAXINPUT, stdin) == NULL) {
            sys_error();
            continue;            
        }
        // block sigint
        sigprocmask(SIG_BLOCK, &sigint_set, NULL);
        yybuf = yy_scan_string(input);
        parse_error = 0;
        yyparse();
        yy_delete_buffer(yybuf);
        if(parse_error) {
            continue;
        }
        interpret(&cmds_root, input);
        free_root(&cmds_root);
    }
}

static void print_prompt() {
    printf("%s@%s: %s $ ", username, hostname, cwd);
}

static void init_prompt() {
    if(gethostname(hostname, MAXHOSTNAMELEN)) {
        sys_error();
    }
    if(getcwd(cwd, MAXPATHLEN) == NULL) {
        sys_error();
    }
    if((username = getlogin()) == NULL) {
        sys_error();
    }
}
