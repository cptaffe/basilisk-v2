typedef struct{
	int type;

	struct {
		int ln;
		int ch;
	};

	union {
		/* Number */
		int n;
		/* Char */
		char c;
		/* String */
		char *str;
		/* Identifier | Operator */
		struct {
			char *name;
			void *val;
		};
	};
} Obj;

/* terms defined in the Language Spec */
enum {
	BASCHAR,
	BASSTRING,
	BASID,
	/* numbers */
	BASNUM,
	/* list */
	BASBLIST,
	BASELIST,
	/* operator */
	BASOPERATOR,
};