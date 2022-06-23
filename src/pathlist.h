/*
 * requires: "csv.h"
 */


typedef struct Pathlist{
	int used;
	char path[CSV_FIELD];
	struct Pathlist *next;
}Pathlist;


void pladd(Pathlist *, char *);