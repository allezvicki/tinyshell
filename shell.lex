%{
#include<stdio.h>

#include "variables.h"
#include "strings.h"
#include "parsetypes.h"
/* yacc -d */
#include "y.tab.h"
#include "error.h"


#define MAX_STR_CONST 1024

char string_buf[MAX_STR_CONST];
char* string_buf_ptr;

void assemble(char c);
void replace_with_var(const char* text);


%}

WS [ \t\n]+
VAR [a-zA-Z_][a-zA-Z_0-9]+
WORD [^ \t\n\<\>\|\&]+
SPECIAL [\<\>\|&]

/* Declare start conditon */
%x STRING

/* make cc happy */
%option noyywrap

%%

{WS} {}
{SPECIAL} {
    return *yytext;
}

${VAR} {
    char* var;
    var = getvar(yytext + 1);
    if(var == NULL) {
        yylval.str = NULL;
    } else {
        yylval.str = add_string(var);
    }
    return STR;
}

<INITIAL>\" {
    BEGIN(STRING);
    string_buf_ptr = string_buf;
}

<STRING>${VAR} {
    // dealing with vars
    replace_with_var(yytext);
}
<STRING>\" { 
    BEGIN(INITIAL);
    assemble('\0');
    yylval.str = add_string(string_buf);
    return STR;
}
<STRING>. {
    // assemble the string literal
    assemble(*yytext);
}

{WORD} {
    yylval.str = add_string(yytext);
    return STR;
}

%%
void assemble(char c) {
    if(string_buf_ptr < string_buf + 1024) {
        *string_buf_ptr = c;
        string_buf_ptr++;
    } else {
        fatal_error("string const too long");
    }
}

void replace_with_var(const char* text) {
    char* var = getvar(text + 1);
    if(var == NULL) {
        return;
    }
    for(int i = 0; var[i] != '\0'; i++) {
        assemble(var[i]);
    }
}
