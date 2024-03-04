#include <stdio.h>

#include "parsetypes.h"
#include "lex.yy.h"

extern int yyparse();
extern root_t cmds_root;

char s[1024];

int main() {
    while(1) {
        fgets(s, 1024, stdin);
        yy_scan_string(s);
        // lex test
        //while (yylex() != 0) {
        //}

        // parse test
        printf("string s is: %s\n", s);
        yyparse();
        printf("parsing done, print root\n");
        /* got cmds_root */
        print_root(&cmds_root);
        printf("free root\n");
        free_root(&cmds_root);
    }
}
