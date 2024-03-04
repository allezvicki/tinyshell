#include <malloc.h>
#include <stdio.h>

#include <string.h>
#include "parsetypes.h"
#include "error.h"

void add_arg(cmd_t* cmd, char* arg) {
    list_item* list_ptr;
    list_ptr = (list_item*) malloc(sizeof(list_item));
    if(list_ptr == NULL) {
        sys_fatal_error();
    }
    list_ptr->next = NULL;
    list_ptr->ptr = arg;
    if(cmd->arglist_head == NULL) {
        cmd->arglist_head = list_ptr;
        cmd->arglist_tail = list_ptr;
    } else {
        cmd->arglist_tail->next = list_ptr;
        cmd->arglist_tail = list_ptr;
    }
    cmd->argnum++;
}
void add_cmd(cmds_t* cmds, cmd_t* cmd){
    list_item* list_ptr;
    cmd_t* cmd_ptr;

    list_ptr = (list_item*) malloc(sizeof(list_item));
    if(list_ptr == NULL) {
        sys_fatal_error();
    }
    list_ptr->next = NULL;
    cmd_ptr = malloc(sizeof(cmd_t));
    if(cmd_ptr == NULL) {
        sys_fatal_error();
    }
    memcpy(cmd_ptr, cmd, sizeof(cmd_t));
    list_ptr->ptr = cmd_ptr;

    if(cmds->cmdlist_head == NULL) {
        cmds->cmdlist_head = list_ptr;
        cmds->cmdlist_tail = list_ptr;
    } else {
        cmds->cmdlist_tail->next = list_ptr;
        cmds->cmdlist_tail = list_ptr;
    }
}

void print_cmd(cmd_t* cmd) {
    list_item* list_ptr;
    printf("    %s if=%s of=%s\n", cmd->cmdname, cmd->infile, cmd->outfile);
    list_ptr = cmd->arglist_head;
    if(list_ptr == NULL) {
    }
    while(list_ptr != NULL) {
        printf("        %s\n", (char*)(list_ptr->ptr));
        list_ptr = list_ptr->next;
    }
}

void print_cmds(cmds_t* cmds) {
    list_item* list_ptr;
    printf("    cmds:\n");
    list_ptr = cmds->cmdlist_head;
    while(list_ptr != NULL) {
        print_cmd((cmd_t*)(list_ptr->ptr));
        list_ptr = list_ptr->next;
    }
}

void print_root(root_t* root) {
    printf("root: %s\n", root->isbg ? "background" : "foreground");
    print_cmds(&root->cmds);
}

void free_cmd(cmd_t* cmd) {
    list_item* list_ptr;
    list_item* old_list_ptr;

    list_ptr = cmd->arglist_head;
    free(cmd->cmdname);
    while(list_ptr != NULL) {
        free(list_ptr->ptr);
        old_list_ptr = list_ptr;
        list_ptr = list_ptr->next;
        free(old_list_ptr);
    }
    free(cmd);
}

void free_cmds(cmds_t* cmds) {
    list_item* list_ptr;
    list_item* old_list_ptr;

    list_ptr = cmds->cmdlist_head;
    while(list_ptr != NULL) {
        free_cmd((cmd_t*)(list_ptr->ptr));
        old_list_ptr = list_ptr;
        list_ptr = list_ptr->next;        
        free(old_list_ptr);
    }
}

void free_root(root_t* root) {
    free_cmds(&root->cmds);
}
