/*
 * requires: hat.h
 */

typedef struct Hvpair{
	char header[HAT_VALLEN];
	char value[HAT_VALLEN];
	int exception; //wether the value should be removed or added
	int filter;
}Hvpair;

int parser_init(int);
int parser_exec(void);
void parser_clean(void);
int parseline(char *);
