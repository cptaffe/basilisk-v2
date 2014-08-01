// lexes based on the Language Spec
#import <stdio.h> // getc & ungetc
#import <stdlib.h> // NULL
#import <stdarg.h>
#import <strings.h>
#import <ctype.h>
#import "tok.h" // token enum values

typedef void (*error) (const char *);

typedef struct {
	FILE *in;
	char *str;
	int len;
	int max;
	int charNum;
	int lineNum;
	int parenDepth;
	error error; // error handler
} Lexer;

// error interface
void errorf (Lexer *l, const char *fmt, ...) {
	va_list ap;
	char str[100];
	if (l->error != NULL) {
		sprintf(str, "\033[1m%d:%d: \033[31merror:\033[0m ", l->lineNum, l->charNum);
		strcat(str, fmt);
		va_start(ap, fmt);
		vsprintf(str, str, ap); va_end(ap);
		strcat(str, "\n");
		l->error(str);
	}
}

// debug
void debug(const char *fmt, ...){
	/*va_list ap;
	char str[100];
	va_start(ap, fmt);
	vsprintf(str, fmt, ap); va_end(ap);
	printf("\033[1m\033[34mdebug:\033[0m %s\n", str);*/
}

// character stuffs
char next(Lexer *l) {
	l->charNum++;
	char c = getc(l->in);
	if (c == '\n'){l->lineNum++; l->charNum = 0;}
	if (l->len - 1 == l->max){errorf(l, "string too long."); exit(1);}
	return l->str[l->len++] = c;
}

int backup(Lexer *l) {
	l->len--; l->charNum--;
	if (l->str[l->len] == '\n'){l->lineNum--;}
	return ungetc(l->str[l->len], l->in);
}

void emit(Lexer *l, int type){
	if (l->len > 0){
		char s[100]; strlcpy(s, l->str, l->len + 1);
		l->len = 0;
		//printf("\033[1m\033[36memit:\033[0m %d: %s\n", type, s);
	}
}

void dump(Lexer *l) {
	l->len = 0;
}

// defines whitespace
int iswhitespace(const char c) {
	return c == ' ' || c == '\t' || c == '\n';
}

// defines letter
int isletter(const char c) {
	return isalpha(c) || c == '_';
}

// defines symbol
int issymbol(const char c) {
	const char symb[] = "~!@#$%^&*-+=";
	for (int i = 0; i < (sizeof symb)/sizeof (char); i++) {
		if (c == symb[i]) {return 1;}
	}
	return 0;
}

// function signature
typedef void *(*lexFunc) (Lexer *);

// lexers only avaliable in this file

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
	debug("lexing program");
	char c;
	while ((c = next(l)) != EOF) {
		if (c == '('){
			emit(l, BASBLIST);
			l->parenDepth++;
			return lexOperator;
		} else if (c == ')') {
			emit(l, BASELIST);
			l->parenDepth--;
			if (l->parenDepth < 0){
				errorf(l, "too many parens");
				l->parenDepth = 0;
			}
			return lexProgram;
		} else if (iswhitespace(c)) {
			emit(l, BASWHITESPACE);
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
	debug("lexing s-expression");
	char c;
	while ((c = next(l)) != EOF) {
		if (isletter(c) || issymbol(c)) {
			// first char is letter
			return lexIdent;
		} else if (isdigit(c)) {
			backup(l);
			return lexNumber;
		} else if (c == ')' || c == '(') {
			backup(l);
			return lexProgram;
		} else if (c == '"') {
			return lexString;
		} else if (c == '\'') {
			return lexChar;
		} else if (isspace(c)) {
			emit(l, BASWHITESPACE);
			return lexSexp;
		} else {
			errorf(l, "unknown character '%c'", c);
			dump(l);
		}
	}
	return NULL;
}

static void *lexIdent (Lexer *l) {
	debug("lexing identifier");
	char c;
	while ((c = next(l)) != EOF) {
		// eat identifier
		if (!isletter(c) && !isdigit(c)) {
			backup(l);
			emit(l, BASID);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexOperator (Lexer *l) {
	debug("lexing operator");
	char c;
	while ((c = next(l)) != EOF) {
		// eat whitespace
		if (!iswhitespace(c)) {
			backup(l);
			emit(l, BASWHITESPACE);
			break;
		}
	}
	// test first character
	if (isletter(c = next(l)) || issymbol(c)) {
		// test following characters
		while ((c = next(l)) != EOF) {
			// eat identifier or symbol
			if ((!isletter(c) && !isdigit(c)) && !issymbol(c)) {
				break;
			}
		}
	} 
	if (c == EOF){return NULL;}
	if (l->len < 2){errorf(l, "list lacks operator");}
	else if (!iswhitespace(c)){errorf(l, "operator not followed by whitespace.");}
	backup(l);
	emit(l, BASOPERATOR);
	return lexSexp;
}

static void *lexString (Lexer *l) {
	debug("lexing string");
	char c;
	while ((c = next(l)) != EOF) {
		// eat string
		if (c == '"') {
			emit(l, BASSTRING);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexChar (Lexer *l) {
	debug("lexing string");
	char c;
	int i = 0;
	while ((c = next(l)) != EOF) {
		// eat char
		i++;
		if (c == '\'') {
			if (i > 2){errorf(l, "character too long");}
			emit(l, BASCHAR);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexNumber (Lexer *l) {
	debug("lexing number");
	char c;
	if ((c = next(l)) == '0') {
		// octal or hexadecimal
		if ((c = next(l)) == 'x' || c == 'X') {
			return lexHexadecimalNumber; // hexadecimal
		} else {
			if (c == EOF){return NULL;} // eof check
			backup(l);
			return lexOctalNumber; // octal
		}
	} else {
		if (c == EOF){return NULL;} // eof check
		return lexDecimalNumber; // decimal
	}
}

static void *lexOctalNumber (Lexer *l) {
	debug("lexing octal");
	char c;
	while ((c = next(l)) != EOF) {
		if (c < '0' || c > '7') {
			// not octal
			backup(l);
			emit(l, BASOCTNUM);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexDecimalNumber (Lexer *l) {
	debug("lexing decimal");
	char c;
	while ((c = next(l)) != EOF) {
		if (c < '0' || c > '9') {
			backup(l);
			emit(l, BASDECNUM);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexHexadecimalNumber (Lexer *l) {
	debug("lexing hexadecimal");
	char c;
	while ((c = next(l)) != EOF) {
		if ((c < '0' || c > '9') && (c < 'A' || c > 'F')) {
			backup(l);
			emit(l, BASHEXNUM);
			return lexSexp;
		}
	}
	return NULL;
}

static void *lexComment (Lexer *l) {
	debug("lexing comment");
	char c;
	while ((c = next(l)) != EOF) {
		if (c == '\n') {
			backup(l); // newline counts as whitespace
			emit(l, BASCOMMENT); // dump comment
			return lexProgram;
		}
	}
	return NULL;
}

int lex (error err) {
	Lexer l;
	l.error = err;
	l.str = calloc(100, sizeof(char));
	l.max = 100;
	l.len = 0;
	l.parenDepth = 0;
	l.charNum = 0; l.lineNum = 1;
	l.in = stdin;
	for (void *state = (void *) lexProgram; state != NULL;) {
		state = ((lexFunc) state)(&l);
	}
	return 0;
}