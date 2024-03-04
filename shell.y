%{

#include "parsetypes.h"
#include "error.h"
extern int yylex();

root_t cmds_root;
int parse_error;

%}

%token <str> STR

%type <cmd> cmd
%type <cmds> cmds
%type <root> root

%union {
    char* str;
    cmd_t cmd;
    cmds_t cmds;
    root_t root;
}

%left '<' '>' '|'
%nonassoc '&'

%%

root : cmds '&' {
                    $$ = (root_t) {$1, 1};
        cmds_root = $$;
      }
      | cmds {
            $$ = (root_t) {$1, 0};
        cmds_root = $$;
      }
      | {
            $$ = (root_t) { {NULL, NULL}, 0};
            cmds_root = $$;
      }
      ;

cmds : cmd {
        $$ = (cmds_t) {NULL, NULL};
        add_cmd(&$$, &$1);
     }
     | cmds '|' cmd {
        add_cmd(&$1, &$3);
        $$ = $1;
     }
     ;

cmd : STR {
        $$ = (cmd_t) {$1, NULL, NULL, NULL, NULL, 0};
    }
    | cmd '<' STR {
        $1.infile = $3;
        $$ = $1;
    }
    | cmd '>' STR {
        $1.outfile = $3;
        $$ = $1;
    }
    | cmd STR {
        add_arg(&$1, $2);
        $$ = $1;
    }
    ;

%%

void yyerror(const char* msg) {
    error("parse error");
    // FIXME Allocated memory won't be freed.
    // Support better error handling.
    parse_error = 1;
}
