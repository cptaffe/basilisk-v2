typedef struct {
	void **stack;
	int len;
	int max;
} stack;

const int STACKBUF = 5;

stack *initstack() {
	stack *stack = malloc(sizeof (stack));
	if (stack == NULL){return NULL;}

	stack->stack = NULL;
	stack->len = 0;
	stack->max = 0;
	return stack;
}

int freestack(stack *stack) {
	free(stack->stack);
	free(stack);
	return 0;
}

int resizestack (stack *stack) {
	int grow, shrink;
	grow = stack == NULL || stack->len >= stack->max;
	shrink = (stack->max - stack->len) > STACKBUF;
	if (!grow && !shrink) {return 0;}
	int buff;
	if (grow){buff = STACKBUF;}
	else{buff = 0;}
	size_t size = (stack->len + buff) * sizeof (void *);
	stack->stack = (void **) realloc(stack->stack, size);
	if (stack->stack == NULL){free(stack->stack); return 1;}
	stack->max = size / sizeof (void *);
	return 0;
}

void *pop (stack *stack) {
	if (stack->len <= 0){return NULL;}
	if(resizestack(stack)){return NULL;}
	stack->len--;
	if (stack->len < 0){return NULL;}
	return stack->stack[stack->len];
}

int push (stack *stack, void *v) {
	if(resizestack(stack)){return 1;}
	stack->len++;
	if (stack->len <= 0){return 1;}
	if (v == NULL) {return 1;}
	stack->stack[stack->len - 1] = v;
	return 0;
}

int resetstack(stack *stack) {
	stack->len = 0;
	return 0;
}