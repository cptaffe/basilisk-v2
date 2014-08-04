
typedef void (*errorHandler)(int, int, int, const char *fmt, ...);

enum{
	ERRERROR,
	ERRWARN,
	ERRNOTE,
};