#include <signal.h>
#include <setjmp.h>
extern sigset_t sigchld_set;
extern sigset_t sigint_set;
extern sigset_t empty_set;
extern sigjmp_buf env;
void siginit();
void siginit_child();
