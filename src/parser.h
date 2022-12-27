/*
 * requires: hat.h
 */

typedef struct Hvpair{
	char header[1024];
	char value[1024];
	int exception; //wether the value should be removed or added
	int filter;
}Hvpair;

int parser_init(int);
int parser_exec(void);
void parser_clean(void);
int parseline(char *);
