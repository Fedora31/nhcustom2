/*
 * requires: "csv.h"
 */

typedef struct Hvpair{
	char header[CSV_FIELD];
	char value[CSV_FIELD];
	int exception; //wether the value should be removed or added
}Hvpair;

int parser_init(Csv *, int);
int parser_exec(void);
int parser_show(void);
void parser_clean(void);
int parseline(char *);
