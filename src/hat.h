/*
 * requires: stack.h, csv.h, time.h
 */

#define HAT_KEYLEN 64
#define HAT_VALLEN 2048
#define HAT_PATHLEN 2048 //must be >= HAT_VALLEN

typedef struct Pty{
	const char *key;
	const char *val;
}Pty;

typedef struct Hat{
	Stack ptys;
	Stack paths;
	time_t date;
}Hat;

int hat_init(void);
int hat_getptyi(char *);
int hat_defsearch(Stack *, char *, char *);
int hat_datesearch(Stack *, time_t, time_t);
int hat_pathsearch(Stack *, char *);
Hat *hat_get(int);
void hat_free(void);
