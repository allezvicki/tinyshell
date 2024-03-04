#include <stdio.h>
#include <stdlib.h>

void fatal_error(const char* msg) {
    fprintf(stderr, "shell: %s\n", msg);
    exit(1);
}

void sys_fatal_error() {
    perror("shell:");
    exit(1);
}

void error(const char* msg) {
    fprintf(stderr, "shell: %s\n", msg);
}

void sys_error() {
    perror("shell");
}
