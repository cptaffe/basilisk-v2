#import "err.h"
#import "tok.h"
#import "stack.h"

void *parseStack(stack *stack, errorHandler err) {
	Obj *tok;
	for (int i = 0; i < stack->len; i++){
		tok = stack->stack[i];
		switch (tok->type) {
			case BASCHAR:
				printf("char{'%c'}\n", tok->c);
				break;
			case BASSTRING:
				printf("string{\"%s\"}\n", tok->str);
				break;
			case BASID:
				printf("id{name: %s}\n", tok->name);
				break;
			case BASNUM:
				printf("num{%d}\n", tok->n);
				break;
			case BASOPERATOR:
				printf("op{name: %s}\n", tok->name);
				break;
			case BASBLIST:
				printf("{\n");
				break;
			case BASELIST:
				printf("}\n");
				break;
		}
	}
	stack->len = 0;
	return 0;
}