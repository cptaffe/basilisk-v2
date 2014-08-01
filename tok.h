// token header

typedef struct{
	char *str;
	int type;
	int ch;
	int len;
} token;

// terms defined in the Language Spec
enum {
	BASWHITESPACE,
	BASCHAR,
	BASSTRING,
	BASID,
	// numbers
	BASOCTNUM,
	BASDECNUM,
	BASHEXNUM,
	// list
	BASBLIST,
	BASELIST,
	// operator
	BASOPERATOR,
	BASCOMMENT,
};