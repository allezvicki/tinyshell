#include <malloc.h>
#include <string.h>

#include "error.h"

// int num_string = 0;

char* add_string(char* s) {
    char* new_string;
    new_string = (char*) malloc(sizeof(char) * (strlen(s) + 1));
    if(new_string != NULL) {
        strcpy(new_string, s);
        return new_string;
    } else {
        sys_fatal_error();
    }
}
