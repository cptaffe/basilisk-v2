/* lexes based on the Language Spec */
#import <stdio.h> /* getc & ungetc */
#import <stdlib.h> /* NULL */
#import <strings.h>
#import <ctype.h>
#import "tok.h" /* token enum values */
#import "err.h"
#import "stack.h"
#import "limits.h"

// lexer error macro
#define errorf(L, ...) L->error(ERRERROR, L->lineNum, L->charNum, __VA_ARGS__);

typedef struct {
	FILE *in;
	char *str;
	int len;
	int max;
	int charNum;
	int lineNum;
	int parenDepth;
	errorHandler error; /* error handler */
	stack *tok;
} Lexer;

// definition functions
static inline int iswhitespace(const char c) {
	return c == ' ' || c == '\t' || c == '\n';}
static inline int isletter(const char c) {
	return isalpha(c) || c == '_';}
static inline int issymbol(const char c) {
	return strchr("~!@#$%^&*-+=", c) != NULL;}

/* character stuffs */
static char next(Lexer *l) {
	l->charNum++;
	char c = getc(l->in);
	if (c == '\n'){l->lineNum++; l->charNum = 0;}
	if (l->len - 1 == l->max){errorf(l, "string too long."); exit(1);}
	return l->str[l->len++] = c;
}

static int backup(Lexer *l) {
	l->len--; l->charNum--;
	if (l->str[l->len] == '\n'){l->lineNum--;}
	return ungetc(l->str[l->len], l->in);
}

static void emit(Lexer *l, Obj *tok){
	l->len = 0;
	tok->ln = l->lineNum;
	tok->ch = l->charNum;
	push(l->tok, tok);
}

static inline void dump(Lexer *l) {l->len = 0;}

typedef void *(*lexFunc) (Lexer *);

static void *lexSexp (Lexer *l);
static void *lexOperator (Lexer *l);
static void *lexIdent (Lexer *l);
static void *lexString (Lexer *l);
static void *lexChar (Lexer *l);
static void *lexNumber (Lexer *l);
static void *lexOctalNumber (Lexer *l);
static void *lexDecimalNumber (Lexer *l);
static void *lexHexadecimalNumber (Lexer *l);
static void *lexComment (Lexer *l);

static void *lexProgram (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		if (c == '('){
			Obj *tok = malloc(sizeof (Obj));
			tok->type = BASBLIST;
			emit(l, tok);
			l->parenDepth++;
			return lexOperator;
		} else if (c == ')') {
			Obj *tok = malloc(sizeof (Obj));
			tok->type = BASELIST;
			emit(l, tok);
			l->parenDepth--;
			if (l->parenDepth < 0){
				errorf(l, "too many parens");
				l->parenDepth = 0;
			}
			if (l->parenDepth == 0){return lexProgram;}
			return lexProgram;
		} else if (iswhitespace(c)) {
			return lexProgram;
		} else if (c == ';') {
			return lexComment;
		} else {
			errorf(l, "unknown character '%c'", c);
			dump(l);
		}
	}
	return NULL;
}

static void *lexSexp (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		if (isletter(c) || issymbol(c)) {
			return lexIdent;
		} else if (isdigit(c)) {
			backup(l);
			return lexNumber;
		} else if (c == ')' || c == '(') {
			backup(l);
			return lexProgram;
		} else if (c == '"') {
			dump(l);
			return lexString;
		} else if (c == '\'') {
			dump(l);
			return lexChar;
		} else if (isspace(c)) {
			dump(l);
			return lexSexp;
		} else {
			errorf(l, "unknown character '%c'", c);
			dump(l);
		}
	}
	return NULL;
}

static void *lexIdent (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		/* eat identifier */
		if (!isletter(c) && !isdigit(c)) {
			backup(l);
			if (!iswhitespace(c)){
				errorf(l, "unknown character in id '%c'", c);
				while (!iswhitespace(c = next(l)) && c != EOF){}
				backup(l); dump(l);
			} else {
				Obj *tok = malloc(sizeof (Obj));
				tok->type = BASID;
				tok->name = calloc(l->len + 1, sizeof (char));
				strlcpy(tok->name, l->str, l->len + 1);
				emit(l, tok);
			}
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexOperator (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		/* eat whitespace */
		if (!iswhitespace(c)) {
			backup(l);
			dump(l);
			break;
		}
	}
	/* test first character */
	if (isletter(c = next(l)) || issymbol(c)) {
		/* test following characters */
		while ((c = next(l)) != EOF) {
			/* eat identifier or symbol */
			if ((!isletter(c) && !isdigit(c)) && !issymbol(c)) {
				backup(l);
				if (l->len < 1){
					errorf(l, "list lacks operator");
					dump(l);
				}
				if (!iswhitespace(c)) {
					errorf(l, "unknown character in op '%c'", c);
					dump(l);
				} else {
					Obj *tok = malloc(sizeof (Obj));
					tok->type = BASOPERATOR;
					tok->name = calloc(l->len + 1, sizeof (char));
					strlcpy(tok->name, l->str, l->len + 1);
					emit(l, tok);
				}
				return lexSexp;
			}
		}
	} else {
		backup(l);
		errorf(l, "list lacks operator");
		dump(l);
		return lexSexp;
	}
	return NULL;
}

static void *lexString (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		/* eat string */
		if (c == '"') {
			Obj *tok = malloc(sizeof (Obj));
			tok->type = BASSTRING;
			tok->str = calloc(l->len, sizeof (char));
			strlcpy(tok->str, l->str, l->len);
			emit(l, tok);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexChar (Lexer *l) {
	char c;
	c = next(l);
	if (next(l) == '\'') {
		Obj *tok = malloc(sizeof (Obj));
		tok->type = BASCHAR;
		tok->c = c;
		emit(l, tok);
	} else {
		errorf(l, "character too long");
		while((c = next(l)) != '\'' && c != EOF){
			/* eat exessive chars */
		}
		dump(l);
	}
	return lexSexp;
}

static void *lexNumber (Lexer *l) {
	char c;
	if ((c = next(l)) == '0') {
		/* octal or hexadecimal */
		if ((c = next(l)) == 'x' || c == 'X') {
			return lexHexadecimalNumber; /* hexadecimal */
		} else {
			if (c == EOF){return NULL;} /* eof check */
			backup(l);
			return lexOctalNumber; /* octal */
		}
	} else {
		if (c == EOF){return NULL;} /* eof check */
		return lexDecimalNumber; /* decimal */
	}
}

static void *lexOctalNumber (Lexer *l) {
	char c; dump(l);
	while ((c = next(l)) != EOF) {
		if (c < '0' || c > '7') {
			/* not octal */
			backup(l);
			/* convert octal to number */
			long int n = strtol(l->str, NULL, 8);
			if (n > INT_MAX){errorf(l, "%d is too big", n); dump(l);}
			else {
				Obj *tok = malloc(sizeof (Obj));
				tok->type = BASNUM;
				tok->n = n;
				emit(l, tok);
			}
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexDecimalNumber (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		if (c < '0' || c > '9') {
			backup(l);
			/* convert octal to number */
			long int n = strtol(l->str, NULL, 10);
			if (n > INT_MAX){errorf(l, "%d is too big", n); dump(l);}
			else {
				Obj *tok = malloc(sizeof (Obj));
				tok->type = BASNUM;
				tok->n = n;
				emit(l, tok);
			}
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexHexadecimalNumber (Lexer *l) {
	char c; dump(l);
	while ((c = next(l)) != EOF) {
		if ((c < '0' || c > '9') && (c < 'A' || c > 'F')) {
			backup(l);
			/* convert octal to number */
			long int n = strtol(l->str, NULL, 16);
			if (n > INT_MAX){errorf(l, "%d is too big", n); dump(l);}
			else {
				Obj *tok = malloc(sizeof (Obj));
				tok->type = BASNUM;
				tok->n = n;
				emit(l, tok);
			}
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexComment (Lexer *l) {
	char c;
	while ((c = next(l)) != EOF) {
		if (c == '\n') {
			backup(l); /* newline counts as whitespace */
			dump(l);
			return lexProgram;
		}
	}
	return NULL;
}

void *initLexer(FILE *in, stack *stack, errorHandler err) {
	Lexer *l = malloc(sizeof (Lexer));
	l->error = err;
	l->str = calloc(100, sizeof (char));
	l->max = 100;
	l->len = 0;
	l->parenDepth = 0;
	l->charNum = 0; l->lineNum = 1;
	l->in = in;
	l->tok = stack;
	return l;
}

/* lexes a statement */
int lexStatement (Lexer *l) {
	for (void *state = (void *) lexProgram; state != NULL;) {
		state = ((lexFunc) state)(l);
	}
	return 0;
}