CFLAGS = -g -fsanitize=address

YYSRCS = y.tab.c y.tab.h lex.yy.c lex.yy.h
PARSE_OBJS = lex.yy.o y.tab.o parsetypes.o strings.o variables.o

SHELL_OBJS = main.o $(PARSE_OBJS) interpret.o sighandler.o builtin.o jobcontrol.o error.o

CMD_OBJS = cat.o ls.o

CMD_DIR = cmd

PARSETEST_OBJS = parsetest.o $(PARSE_OBJS) error.o

.PHONY: all clean

shell: $(SHELL_OBJS)
	cc $(CFLAGS) -o $@ $^

main.o: main.c
	cc $(CFLAGS) -c $^

main.c: lex.yy.h

lex.yy.c: y.tab.h
# cmd: $(CMD_OBJS)
# 	cd $(CMD_DIR) ; for cmd in $(CMD_OBJS) ; do \
# 		cc -o $${cmd%.o} $$cmd \
# 	done

lex.yy.c lex.yy.h: shell.lex
	flex --header-file=lex.yy.h shell.lex

y.tab.c y.tab.h: shell.y
	yacc -d shell.y

parsetest: $(PARSETEST_OBJS)
	cc $(CFLAGS) -o $@ $^

clean:
	-rm shell *.o $(CMD_DIR)/*.o $(YYSRCS) parsetest

