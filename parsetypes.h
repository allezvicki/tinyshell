#ifndef PARSETYPES_H
#define PARSETYPES_H
typedef struct {
    // head is a pointer to arg or cmd
    void* ptr;
    // next is a pointer to the next list item
    void* next;
} list_item;

typedef struct {
    char* cmdname;
    list_item* arglist_head;
    list_item* arglist_tail;
    char* infile;
    char* outfile;
    int argnum;
} cmd_t;

typedef struct {
    list_item* cmdlist_head;
    list_item* cmdlist_tail;
} cmds_t;

typedef struct {
    cmds_t cmds;
    // is background?
    int isbg;
} root_t;

void add_arg(cmd_t* cmd, char* arg);
void add_cmd(cmds_t* cmds, cmd_t* cmd);
void print_root(root_t* root);
void free_root(root_t* root);
#endif
