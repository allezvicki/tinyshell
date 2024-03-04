# Description
A toy interactive shell with functional job control.

Trying to be as POSIX compliant as possible. Tested on FreeBSD and Linux.

Frontend with yacc and flex.

Supported syntax: pipeline '|'; std i/o redirection '<' or '>'; background '&'; double quotes.

Supported builtins: echo, cd, pwd, exit, jobs, fg, bg.

# Build and Run
Yacc and flex should be installed before building.
``` sh
make
./shell
```

# Known bugs
A job that has some procs terminated and some stopped is regarded as running.

Better error handling is needed, allocated parse tree memory will not be freed
after yyparse error.

Local variable support is not yet added. $anyvar will be expanded to nothing.

Job id is not implemented. Use job process group id instead.
