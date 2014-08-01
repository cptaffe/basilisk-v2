// main integrates the error handler
// & symbol table aspects of the compiler
#include "lex.c"

void errorHandler(const char *str){
	printf("%s", str);
}

int main() {
	lex(errorHandler);
}