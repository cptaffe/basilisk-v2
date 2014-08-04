/* main integrates the error handler 
& symbol table aspects of the compiler */
#import <strings.h>
#import <stdarg.h>
#import "lex.c"
#import "parse.c"
#import "err.h"
#import "stack.h"

int errors = 0;

void error(int err, int ln, int ch, const char *fmt, ...){
	int col = 31;
	char type[30], msg[100];
	va_list ap;
	if (err == ERRERROR){
		col = 31;
		strcpy(type, "error");
	} else if (err == ERRWARN){
		col = 35;
		strcpy(type, "warning");
	} else if (err == ERRNOTE){
		col = 36;
		strcpy(type, "note");
	}
	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);
	printf("\033[1m\033[%dm%s \033[0m\033[1m%d.%d\033[0m %s\n", col, type, ln, ch, msg);
	errors++;
}

int main(int argc, char *argv[]) {
	FILE *fp;
	if (argc > 1){
		fp = fopen(argv[1], "r");
		if (fp == NULL){perror("error");}
	} else {
		fp = stdin;
	}
	stack *stack = initstack();
	Lexer *l = initLexer(fp, stack, error);
	char c;
	while((c = getc(fp)) != EOF){
		ungetc(c, fp);
		lexStatement(l);
		if (errors == 0) {
			parseStack(stack, error);
		} else {
			printf("\033[1m\033[30mnote\033[0m too many errors to continue.\n");
		}
	}
}