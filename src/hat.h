/*
 * requires: stack.h, csv.h
 */

#define HAT_KEYLEN 64
#define HAT_VALLEN 1024
#define HAT_PATHLEN 1024 //must be >= HAT_VALLEN

typedef struct Pty{
	char key[HAT_KEYLEN];
	char val[HAT_VALLEN];
}Pty;

typedef struct Hat{
	Stack ptys;
	Stack paths;
}Hat;

int hat_init(void);
int hat_getptyi(char *);
